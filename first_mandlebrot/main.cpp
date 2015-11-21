// Include standard headers
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <map>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../common/shader.hpp"
#include "../common/text2D.hpp"

#include "./mand.hpp"
#include "./text.hpp"

static void framebuffer_cb(GLFWwindow* window, int width, int height);
static void wheel_cb(GLFWwindow* window, double xoffset, double yoffset);

GLFWwindow* window;
unsigned int prog;

bool mousedown;

static void cursor_position_callback(GLFWwindow* window, double xpos,
                                     double ypos);
static void mouse_button_callback(GLFWwindow* window, int button, int action,
                                  int mods);

/// Measure shader execution time, and provide a report.
class TimeGL {
 public:
  TimeGL() : fResult(0.0), first_call(true) {}
  /// Start the timer.
  void Start(void);
  /// Stop the timer.
  void Stop(void);
  // get the time back(usec)
  float Report(void);

 private:
  GLuint query[1];
  float fResult;
  bool first_call;
};

void TimeGL::Start(void) {
  if (first_call) {
    glGenQueries(1, query);
    first_call = false;
    fResult = 0.0;
  }
  glBeginQuery(GL_TIME_ELAPSED, query[0]);
}

void TimeGL::Stop(void) {
  glEndQuery(GL_TIME_ELAPSED);
}

float TimeGL::Report(void) {
  GLuint result;
  glGetQueryObjectuiv(query[0], GL_QUERY_RESULT, &result);
  fResult = result * 0.001;
  return fResult; }

class AtomicCounter {
 public:
  AtomicCounter(void){};
  void init(void);
  void reset(void);
  glm::uvec3 read(void);
  GLuint atomicsBuffer;
  GLuint atomicsReadBuffer;
};

void AtomicCounter::init(void) {
  // declare and generate a buffer object name
  glGenBuffers(1, &atomicsBuffer);
  // bind the buffer and define its initial storage capacity
  glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicsBuffer);
  glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint) * 3, NULL,
               GL_DYNAMIC_DRAW);
  // unbind the buffer
  glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

  glGenBuffers(1, &atomicsReadBuffer);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, atomicsReadBuffer);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * 3, NULL,
               GL_DYNAMIC_DRAW);

}

void AtomicCounter::reset(void){
  glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicsBuffer);
  GLuint a[3] = {0,0,0};
  glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0 , sizeof(GLuint) * 3, a);
  // glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

}

glm::uvec3 AtomicCounter::read(void) {
  // GLuint *userCounters;
  glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomicsBuffer);
  GLuint userCounters[3];
  glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicsBuffer);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, atomicsReadBuffer);

  glCopyBufferSubData(GL_ATOMIC_COUNTER_BUFFER, GL_SHADER_STORAGE_BUFFER,
    0,0,12);

  glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, 12, userCounters);

  glm::uvec3 ret = glm::uvec3(userCounters[0], userCounters[1], userCounters[2]);
  glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
  glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
  return ret;
}



class RenderContext {
 private:
  int screen_width = 1024;
  int screen_height = 768;
  double pos_x, pos_y;
  double dragstart_x, dragstart_y;

  int iter = 10;
  double cx = -0.7, cy = 0.0;
  double cur_scale = 1.1;
  glm::dvec2 get_xy(double x, double y);

  TextGL text;
  TimeGL tm;
  AtomicCounter ac;
  Mandlebrot mb;
  char hud[256] = {0};
  float aspect_ratio;

 public:
  RenderContext(void);
  int render(void);
  void reshape(GLFWwindow* window, int width, int height);
  void zoom(double yoffset);
  void startmove(void);
  void mouseposition(double x, double y);
  int changed = 0;
  ~RenderContext(void);

};

RenderContext::RenderContext() {
  if (!glfwInit()) {
    fprintf(stderr, "Failed to initialize GLFW\n");
    exit(-1);
  }

  // Initialise GLFW
  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  screen_width = 1024;
  screen_height = 768;
  aspect_ratio = float(screen_width) / float(screen_height);
  // Open a window and create its OpenGL context
  window = glfwCreateWindow(screen_width, screen_height, "mandlebroh", NULL,
                            NULL);

  if (window == NULL) {
    fprintf(stderr,
            "Failed to open GLFW window. If you have an Intel GPU, they are "
            "not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
    glfwTerminate();
    exit(-1);
  }
  glfwMakeContextCurrent(window);

  // Initialize GLEW
  glewExperimental = true;  // Needed for core profile

  if (glewInit() != GLEW_OK) {
    fprintf(stderr, "Failed to initialize GLEW\n");
    exit(-1);
  }

  // Ensure we can capture the escape key being pressed below
  glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
  glfwSetCursorPos(window, screen_width / 2, screen_height / 2);

  // setup the callbacks
  glfwSetFramebufferSizeCallback(window, framebuffer_cb);
  glfwSetScrollCallback(window, wheel_cb);
  glfwSetCursorPosCallback(window, cursor_position_callback);
  glfwSetMouseButtonCallback(window, mouse_button_callback);

  // init cursor pos
  glfwGetCursorPos(window, &pos_x, &pos_y);
  glfwGetCursorPos(window, &dragstart_x, &dragstart_y);

  glfwSwapInterval(1);
  mb.init();
  ac.init();
  text = TextGL("stuff", 20, glm::vec3(.75, .75, 1));
}

int RenderContext::render(void) {

  tm.Start();
  glm::uvec3 counters = glm::uvec3(0);
  ac.reset();
  glfwGetCursorPos(window, &pos_x, &pos_y);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  mb.render(iter, aspect_ratio, cx, cy, cur_scale);
  text.print(hud, 10, 50, screen_width, screen_height);

  counters = ac.read();

  tm.Stop();
  float time = tm.Report();
  glfwSwapBuffers(window);
  glfwPollEvents();

  float mod = glm::clamp(16000.0f / time, .1f, 2.0f);
  iter = glm::clamp(int(iter * mod), 10, 50000) ;
  // iter = 100;
  snprintf(hud, sizeof(hud), "%5.2fms, %5E, %5i,\n%5.3E, %6f, %6f \n",
           time/1000, double(counters[0]), iter, cur_scale, cx, cy);
  // printf("%s\n", hud);

  return 0;
}

glm::dvec2 RenderContext::get_xy(double x, double y) {
  auto u = (x / screen_width);
  auto v = (1 - y / screen_height);

  auto posx = aspect_ratio * (2*u - 1) * cur_scale + cx;
  auto posy = (2*v - 1) * cur_scale + cy;

  // printf("\n %5f %5f %5f %5f %5.3f %5.3f\n", u, v, posx, posy, cx, cy);

  return glm::dvec2(posx, posy);
}

void RenderContext::zoom(double yoffset) {
  auto old_scale = cur_scale;
  auto center = glm::dvec2(cx, cy);
  auto pos = get_xy(pos_x, pos_y);

  cur_scale *= (yoffset < 0 ? 1.1 : (1 / 1.1));

  auto new_center = pos + (cur_scale / old_scale) * (center - pos);

  cx = new_center.x;
  cy = new_center.y;
  // printf("\n cx %6f cy %6f xpos %6f ypos %6f xsize %6i ysize %6i posx %6f
  // posy %6f \n",
  //    cx, cy, xpos, ypos, xsize, ysize, pos.x, pos.y);
}

void RenderContext::startmove(void) {
  dragstart_x = pos_x;
  dragstart_y = pos_y;
}

void RenderContext::reshape(GLFWwindow* window, int width, int height) {
  glViewport(0, 0, (GLint)width, (GLint)height);
  aspect_ratio = float(width) / float(height);
  screen_width = width;
  screen_height = height;
  render();
}

void RenderContext::mouseposition(double x, double y) {
  if (mousedown) {
    auto new_center =
        glm::dvec2(cx, cy) + get_xy(dragstart_x, dragstart_y) - get_xy(x, y);
    cx = new_center.x;
    cy = new_center.y;
    dragstart_x = x;
    dragstart_y = y;
  }
  // auto pp = get_xy(x, y);
  // printf("\n %5f %5f\n", pp.x, pp.y);
}

RenderContext::~RenderContext(void) {
}

RenderContext context;

static void framebuffer_cb(GLFWwindow* window, int width, int height) {
  context.reshape(window, width, height);
}

static void wheel_cb(GLFWwindow* window, double xoffset, double yoffset) {
  context.zoom(yoffset);
}

static void cursor_position_callback(GLFWwindow* window, double xpos,
                                     double ypos) {
  context.mouseposition(xpos, ypos);
}

static void mouse_button_callback(GLFWwindow* window, int button, int action,
                                  int mods) {
  if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS) {
    mousedown = true;
    context.startmove();
  }
  if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_RELEASE) {
    mousedown = false;
  }
}

int main(void) {
  do {
    context.render();
  }  // Check if the ESC key was pressed or the window was closed
  while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
         glfwWindowShouldClose(window) == 0);

  // Close OpenGL window and terminate GLFW
  glfwTerminate();
  return 0;
}
