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
using namespace glm;
using namespace std;

static void framebuffer_cb(GLFWwindow* window, int width, int height);
static void wheel_cb(GLFWwindow* window, double xoffset, double yoffset);

#define VERSION 330

GLFWwindow* window;
unsigned int prog;

bool mousedown;

GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path){

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
  GLuint ProgramID = glCreateProgram();

  GLint Result = GL_FALSE;
  int InfoLogLength;

  if (nullptr != vertex_file_path) {
    // Read the Vertex Shader code from the file
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
    if (VertexShaderStream.is_open()) {
      std::string Line = "";
      while (getline(VertexShaderStream, Line))
        VertexShaderCode += "\n" + Line;
      VertexShaderStream.close();
    }
    else {
      printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path);
      getchar();
      return 0;
    }

    // Compile Vertex Shader
    printf("Compiling shader : %s\n", vertex_file_path);
    char const * VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
    glCompileShader(VertexShaderID);

    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0) {
      std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
      glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
      printf("%s\n", &VertexShaderErrorMessage[0]);
    }

    glAttachShader(ProgramID, VertexShaderID);

  }

  if (nullptr != fragment_file_path) {
    // Read the Fragment Shader code from the file
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
    if (FragmentShaderStream.is_open()) {
      std::string Line = "";
      while (getline(FragmentShaderStream, Line))
        FragmentShaderCode += "\n" + Line;
      FragmentShaderStream.close();
    }
    // Compile Fragment Shader
    printf("Compiling shader : %s\n", fragment_file_path);
    char const * FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
    glCompileShader(FragmentShaderID);

    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0) {
      std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
      glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
      printf("%s\n", &FragmentShaderErrorMessage[0]);
    }
    glAttachShader(ProgramID, FragmentShaderID);
  }

	// Link the program
	printf("Linking program\n");
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> ProgramErrorMessage(InfoLogLength+1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

void set_uniform1f(unsigned int prog, const char *name, float val) {
  int loc = glGetUniformLocationARB(prog, name);
  if (loc != -1) {
    glUniform1f(loc, val);
  }
}
void set_uniform1d(unsigned int prog, const char *name, double val) {
  int loc = glGetUniformLocationARB(prog, name);
  if (loc != -1) {
    glUniform1d(loc, val);
  }
}
void set_uniform2f(unsigned int prog, const char *name, float v1, float v2) {
  int loc = glGetUniformLocationARB(prog, name);
  if (loc != -1) {
    glUniform2f(loc, v1, v2);
  }
}
void set_uniform2d(unsigned int prog, const char *name, double v1, double v2) {
  int loc = glGetUniformLocationARB(prog, name);
  if (loc != -1) {
    glUniform2d(loc, v1, v2);
  }
}

void set_uniform1i(unsigned int prog, const char *name, int val) {
  int loc = glGetUniformLocationARB(prog, name);
  if (loc != -1) {
    glUniform1i(loc, val);
  }
}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);



/// Measure shader execution time, and provide a report.
class TimeMeasure {
public:
  /// Use this to create a static instance.
  /// @param title A string used in the report.
  TimeMeasure() : fQuery(0), fFirst(true) {}

  /// Start the timer.
  void Start() {
    if (fFirst) {
      glGenQueries(1, &fQuery);
      fFirst = false;
      fresult = 0.0;
    }
    else {
      GLuint result;
      glGetQueryObjectuiv(fQuery, GL_QUERY_RESULT, &result);
      fresult = result * 0.001;
    }
    glBeginQuery(GL_TIME_ELAPSED, fQuery);
  }

  /// Stop the timer.
  void Stop(void) {
    glEndQuery(GL_TIME_ELAPSED);
  }

  float Report(void);
private:
  GLuint fQuery;
  float fresult;
  bool fFirst;
};

float TimeMeasure::Report(void) {
  return fresult;
}

class RenderContext {
private:
  glm::dvec2 get_xy(double x, double y);
  int screen_width = 1024;
  int screen_height = 768;
  GLuint VertexArrayID;
  GLuint programID;

  GLuint vertexbuffer;
  GLuint uvbuffer;
  GLuint TextureLoc;
  GLuint TextureID;
  GLuint MatrixID, iterID;
  glm::mat4 Projection;
  int fast_iter = 3000, iter = 3000;

  double cx = -0.7, cy = 0.0;
  double cur_scale = 1;
  float aspect_ratio;
  int dragstart_x, dragstart_y;
  int pos_x, pos_y;
public:
  RenderContext(void);
  int render(void);
  void reshape(GLFWwindow* window, int width, int height);
  void zoom(double yoffset);
  void startmove(void);
  void mouseposition(double x, double y);
  int changed = 0;
  ~RenderContext(void);
  TimeMeasure tm;
};

RenderContext::RenderContext() {
  if (!glfwInit())
  {
    fprintf(stderr, "Failed to initialize GLFW\n");
    exit(-1);
  }

  // Initialise GLFW
  glfwWindowHint(GLFW_SAMPLES, 1);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  screen_width = 1024;
  screen_height = 768;

  // Open a window and create its OpenGL context
  window = glfwCreateWindow(screen_width, screen_height, "messin around", NULL, NULL);

  if (window == NULL) {
    fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
    glfwTerminate();
    exit(-1);
  }
  glfwMakeContextCurrent(window);

  // Initialize GLEW
  glewExperimental = true; // Needed for core profile
  if (glewInit() != GLEW_OK) {
    fprintf(stderr, "Failed to initialize GLEW\n");
    exit(-1);
  }

  // Ensure we can capture the escape key being pressed below
  glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
  glfwSetCursorPos(window, screen_width / 2, screen_height / 2);

  glfwSetFramebufferSizeCallback(window, framebuffer_cb);
  glfwSetScrollCallback(window, wheel_cb);
  glfwSetCursorPosCallback(window, cursor_position_callback);
  glfwSetMouseButtonCallback(window, mouse_button_callback);
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

  glfwSwapInterval(0);
  Projection = glm::ortho(1.0f, 1.0f, 1.0f, -1.0f);

  const GLfloat g_vertex_buffer_data[] = {
    -1,-1, 0, 1,
    -1, 1, 0, 1,
    1,-1, 0, 1,
    1, 1, 0, 1,
  };

  glGenBuffers(1, &vertexbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

  const GLfloat g_textcoords[] = {
    0.0, 0.0,
    0.0, 1.0,
    1.0, 0.0,
    1.0, 1.0
  };
  glGenBuffers(1, &uvbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_textcoords), g_textcoords, GL_STATIC_DRAW);
  int width, height;
  glfwGetWindowSize(window, &width, &height);
  aspect_ratio = float(width) / float(height);

  const GLubyte g_colors[]{
    0xff, 0x33, 0, 0,
    0x33, 0x33, 0xff, 0,
    0xff, 0x33, 0x33, 0,
    0x33, 0x88, 0xff, 0,
      };

  // Create one OpenGL texture

  glGenTextures(1, &TextureID);
  // "Bind" the newly created texture : all future texture functions will modify this texture
  glBindTexture(GL_TEXTURE_1D, TextureID);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  // Give the image to OpenGL

  glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE, g_colors);

  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  TextureLoc = glGetUniformLocation(programID, "colors");


}


int RenderContext::render(void)
{
  iter = fast_iter;
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glUseProgram(programID);

  set_uniform1i(programID, "iter", iter);
  set_uniform1f(programID, "aspect", aspect_ratio);
#if VERSION == 330
  set_uniform2f(programID, "center", float(cx), float(cy));
  set_uniform1f(programID, "scale", float(cur_scale));
#else
  set_uniform2d(programID, "center", cx, cy);
  set_uniform1d(programID, "scale", cur_scale);
#endif
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_1D, TextureID);
  glUniform1i(TextureLoc, 0);

  glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &Projection[0][0]);

  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
  glVertexAttribPointer(
    0,
    4,
    GL_FLOAT,
    GL_FALSE,
    0,
    (void*)0
    );

  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
  glVertexAttribPointer(
    1,
    2,
    GL_FLOAT,
    GL_FALSE,
    0,
    (void*)0
    );

  tm.Start();

  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);

  tm.Stop();
  glfwSwapBuffers(window);

  glfwPollEvents();

  float time = tm.Report();

  float mod = glm::clamp(16000.0f / time, .9f, 1.1f);
  fast_iter = glm::clamp(int(fast_iter * mod), 10, 1000);
  printf("\r %5f  %7.2f, %7.2f, %7i, %7E                         ", aspect_ratio,  time, mod, iter, cur_scale);

  return 0;

}

glm::dvec2 RenderContext::get_xy(double x, double y) {
  int xsize,ysize;
  glfwGetFramebufferSize(window, &xsize, &ysize);

  auto posx = cx + cur_scale * 2 * ((x/xsize-.5)),
       posy = cy - cur_scale * 2 * ((y/ysize-.5)/aspect_ratio);

  printf("\n%5f %5f %5.3f %5.3f\n", posx, posy, cx, cy);

  return glm::dvec2(posx, posy);

}


void RenderContext::zoom(double yoffset) {
  auto old_scale = cur_scale;
  auto center = glm::dvec2(cx,cy);
  auto pos = get_xy(pos_x, pos_y);
  cur_scale *= (yoffset < 0 ? 1.1 : (1/1.1));

  auto new_center = pos + (cur_scale/old_scale) * (center - pos);

  cx = new_center.x;
  cy = new_center.y;
  changed = 0;
  // printf("\n cx %6f cy %6f xpos %6f ypos %6f xsize %6i ysize %6i posx %6f posy %6f \n",
//    cx, cy, xpos, ypos, xsize, ysize, pos.x, pos.y);
}

void RenderContext::startmove(void)
{
  dragstart_x = pos_x;
  dragstart_y = pos_y;
}

void RenderContext::reshape(GLFWwindow* window, int width, int height)
{
  printf("%4i %4i\n", width, height);
  glViewport(0, 0, (GLint)width, (GLint)height);
  aspect_ratio = (float)width / (float)height;
  render();
}

void RenderContext::mouseposition(double x, double y)
{
  pos_x = x; pos_y = y;
  // get_xy(x, y);

}

RenderContext::~RenderContext(void)
{
  glDeleteBuffers(1, &vertexbuffer);
  glDeleteBuffers(1, &uvbuffer);
  glDeleteProgram(programID);
  glDeleteTextures(1, &TextureLoc);
  glDeleteVertexArrays(1, &VertexArrayID);
}

RenderContext context;

static void framebuffer_cb(GLFWwindow* window, int width, int height) {
  context.reshape(window, width, height);
}

static void wheel_cb(GLFWwindow* window, double xoffset, double yoffset) {
  context.zoom(yoffset);
}


static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
  context.mouseposition(xpos, ypos);
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
  if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS) {
    mousedown = true;
    context.startmove();
  }
  if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_RELEASE) {
    mousedown = false;
  }
}


int main( void )
{

  do{
    context.render();
	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );

  	// Close OpenGL window and terminate GLFW
	glfwTerminate();
	return 0;
}

