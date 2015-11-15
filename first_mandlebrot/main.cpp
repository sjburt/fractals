// Include standard headers
#include <stdio.h>
#include <stdlib.h>

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

static void framebuffer_cb(GLFWwindow* window, int width, int height);
static void wheel_cb(GLFWwindow* window, double xoffset, double yoffset);

// #define VERSION 330

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
  GLuint result;
  glGetQueryObjectuiv(query[0], GL_QUERY_RESULT, &result);
  fResult = result * 0.001;
}

float TimeGL::Report(void) { return fResult; }

class AtomicCounter {
 public:
  AtomicCounter(void){};
  void init(void);
  void reset(void);
  glm::uvec3 read(void);
  GLuint atomicsBuffer;
};

void AtomicCounter::init(void) {
  // declare and generate a buffer object name
  glGenBuffers(1, &atomicsBuffer);
  // bind the buffer and define its initial storage capacity
  glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicsBuffer);
  glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint) * 3, NULL,
               GL_DYNAMIC_DRAW);
  // unbind the buffer
  // glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
}

void AtomicCounter::reset(void){
  // glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicsBuffer);
  GLuint a[3] = {0,0,0};
  glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0 , sizeof(GLuint) * 3, a);
  // glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

}

glm::uvec3 AtomicCounter::read(void) {
  GLuint *userCounters;

  glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicsBuffer);
  // again we map the buffer to userCounters, but this time for read-only access
  userCounters = (GLuint*)glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER,
                                         0,
                                         sizeof(GLuint) * 3,
                                         GL_MAP_READ_BIT
                                        );

  // copy the values to other variables because...

  glm::uvec3 ret = glm::uvec3(userCounters[0], userCounters[1], userCounters[2]);
  // ... as soon as we unmap the buffer
  // the pointer userCounters becomes invalid.
  glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
  return ret;
}

class Mandlebrot {
 private:
  GLuint VertexArrayID;
  GLuint programID;

  GLuint vertexbuffer;
  GLuint uvbuffer;
  GLuint TextureLoc;
  GLuint TextureID;
  GLuint MatrixID, iterID;

  GLuint u_iter, u_asp, u_center, u_scale;

  glm::mat4 Projection;


 public:
  Mandlebrot(void);
  void init(void);
  void render(const int iter, const float aspect_ratio, const double cx,
              const double cy, const double scale);
  ~Mandlebrot(void);
};

Mandlebrot::Mandlebrot(void) {

}

void Mandlebrot::init(void) {
  glGenVertexArrays(1, &VertexArrayID);
  glBindVertexArray(VertexArrayID);

// Create and compile our GLSL program from the shaders
#if VERSION == 330
  programID = LoadShaders("passthrough.vert", "mand_single.frag");
#else
  programID = LoadShaders("passthrough.vert", "mand.frag");
#endif

  if (!programID) exit(-1);

  // get a handle for the MVP input.
  MatrixID = glGetUniformLocation(programID, "MVP");
  iterID = glGetUniformLocation(programID, "iter");

  Projection = glm::ortho(1.0f, 1.0f, 1.0f, -1.0f);

  // Make a square.
  const GLfloat g_vertex_buffer_data[][4] = {
      {-1, -1, 0, 1}, {-1, 1, 0, 1}, {1, -1, 0, 1}, {1, 1, 0, 1}};

  glGenBuffers(1, &vertexbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data),
               g_vertex_buffer_data, GL_STATIC_DRAW);

  // These are the u,v coordinates of each vertex.
  const GLfloat g_textcoords[][2] = {
      {-1.0, -1.0}, {-1.0, 1.0}, {1.0, -1.0}, {1.0, 1.0}};

  glGenBuffers(1, &uvbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_textcoords), g_textcoords,
               GL_STATIC_DRAW);

  // Create a 1-d texture to use as a color palette.
  const GLubyte g_colors[][4] = {{0xff, 0x33, 0, 0},
                                 {0x33, 0x33, 0xff, 0},
                                 {0xff, 0x33, 0x33, 0},
                                 {0x44, 0x88, 0xff, 0},
                                 {0x44, 0x44, 0xff, 0},
                                 {0xff, 0x44, 0x44, 0},
                                 {0x33, 0x88, 0xdd, 0},
                                 {0x33, 0x55, 0xdd, 0},
                                 {0xdd, 0x55, 0x33, 0},
                                 {0x33, 0x88, 0xdd, 0},
                                 {0x33, 0x33, 0xff, 0},
                                 {0xdd, 0x44, 0x44, 0},
                                 {0x33, 0x88, 0xff, 0}};

  glGenTextures(1, &TextureID);
  glBindTexture(GL_TEXTURE_1D, TextureID);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 12, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               g_colors);

  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  TextureLoc = glGetUniformLocation(programID, "colors");

  u_iter = glGetUniformLocationARB(programID, "iter");
  u_asp = glGetUniformLocationARB(programID, "aspect");

  u_scale = glGetUniformLocation(programID, "scale");
  u_center = glGetUniformLocationARB(programID, "center");


}

void Mandlebrot::render(const int iter, const float aspect_ratio,
                        const double cx, const double cy,
                        const double cur_scale) {
  glUseProgram(programID);


  glUniform1i(u_iter, iter);
  glUniform1f(u_asp, aspect_ratio);



#if VERSION == 330
  glUniform2f(u_center, float(cx), float(cy));
  glUniform1f(u_scale, float(cur_scale));
#else
  glUniform2d(u_center, cx, cy);
  glUniform1d(u_scale, cur_scale);
#endif

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_1D, TextureID);
  glUniform1i(TextureLoc, 0);

  glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &Projection[0][0]);

  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);

}

Mandlebrot::~Mandlebrot(void) {
  glDeleteBuffers(1, &vertexbuffer);
  glDeleteBuffers(1, &uvbuffer);
  glDeleteProgram(programID);
  glDeleteTextures(1, &TextureLoc);
  glDeleteVertexArrays(1, &VertexArrayID);
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
private:
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
  window = glfwCreateWindow(screen_width, screen_height, "messin around", NULL,
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
  initText2D("Holstein.DDS");
}

int RenderContext::render(void) {
  glfwGetCursorPos(window, &pos_x, &pos_y);
  tm.Start();
  ac.reset();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, ac.atomicsBuffer);

  mb.render(iter, aspect_ratio, cx, cy, cur_scale);
  auto counters = ac.read();
  tm.Stop();

  float time = tm.Report();

  printText2D(hud, 10, 50, 10);

  glfwSwapBuffers(window);
  glfwPollEvents();

  float mod = glm::clamp(16000.0f / time, .66f, 1.5f);
  iter = glm::clamp(int(iter * mod), 10, 50000) ;
  snprintf(hud, sizeof(hud), "%5.2fms, %7E, %7i, %7E, %6f, %6f",
           time/1000, double(counters[0]), iter, cur_scale, cx, cy);
  printf("%s\n", hud);

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
  cleanupText2D();
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
