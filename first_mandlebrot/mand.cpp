// Include GLEW
#include <GL/glew.h>

#include "../common/shader.hpp"
#include "./mand.hpp"

//  #define VERSION 330

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
