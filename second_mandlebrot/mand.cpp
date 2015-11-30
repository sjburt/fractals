// Include GLEW
#include <cstring>
#include <iostream>
#include <iomanip>
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
  init_prog = LoadShaders("passthrough.vert", "init.frag");
  if (!init_prog) exit(-1);

  kern_prog = LoadShaders("passthrough.vert", "mand_kern.frag");
  if (!kern_prog) exit(-1);

  show_prog = LoadShaders("passthrough.vert", "mand_show.frag");
  if (!show_prog) exit(-1);


  // get a handle for the MVP input.

  iterID = glGetUniformLocation(kern_prog, "iter");

  Projection = glm::ortho(1.0f, 1.0f, 1.0f, -1.0f);

  // Make a square.

  glGenBuffers(1, &vertexbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data),
               g_vertex_buffer_data, GL_STATIC_DRAW);

  glGenBuffers(1, &uvbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_textcoords), g_textcoords,
               GL_DYNAMIC_DRAW);

  // Create a 1-d texture to use as a color palette.
  const GLubyte g_colors[][4] = {{0xff, 0x33, 0, 0xff},
                                 {0x33, 0x33, 0xff, 0xff},
                                 {0xff, 0x33, 0x33, 0xff},
                                 {0x44, 0x88, 0xff, 0xff},
                                 {0x44, 0x44, 0xff, 0xff},
                                 {0xff, 0x44, 0x44, 0xff},
                                 {0x33, 0x88, 0xdd, 0xff},
                                 {0x33, 0x55, 0xdd, 0xff},
                                 {0xdd, 0x55, 0x33, 0xff},
                                 {0x33, 0x88, 0xdd, 0xff},
                                 {0x33, 0x33, 0xff, 0xff},
                                 {0xdd, 0x44, 0x44, 0xff},
                                 {0x33, 0x88, 0xff, 0xff}};

  glGenTextures(1, &TextureID);
  glBindTexture(GL_TEXTURE_1D, TextureID);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 12, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               g_colors);

  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  TextureLoc = glGetUniformLocation(kern_prog, "colors");

  handles["cloc"] = glGetUniformLocation(kern_prog, "c");

  u_iter = glGetUniformLocationARB(kern_prog, "iter");
  u_asp = glGetUniformLocationARB(init_prog, "aspect");

  u_scale = glGetUniformLocation(init_prog, "scale");
  u_center = glGetUniformLocationARB(init_prog, "center");

  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // generate a framebuffer to render to

  glGenFramebuffers(1, &handles["fbo_init"]);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, handles["fbo_init"]);
  glGenRenderbuffers(1, &handles["rbo_init"]);
  glBindRenderbuffer(GL_RENDERBUFFER, handles["rbo_init"]);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_RG, 1024, 768);
  glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                            GL_RENDERBUFFER, handles["rbo_init"]);


  glGenFramebuffers(1, &handles["fbo_render"]);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, handles["fbo_render"]);
  glBindRenderbuffer(GL_RENDERBUFFER, handles["rbo_init"]);
  glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                            GL_RENDERBUFFER, handles["rbo_init"]);


  glBindFramebuffer(GL_FRAMEBUFFER, 0);


}

void Mandlebrot::render(const int iter, const float aspect_ratio,
                        const double cx, const double cy,
                        const double cur_scale) {

  glUseProgram(init_prog);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, handles["fbo_init"]);
  glBindRenderbuffer(GL_RENDERBUFFER, handles["rbo_init"]);
  glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                            GL_RENDERBUFFER, handles["rbo_init"]);

  if (int i = glCheckFramebufferStatus(handles["fbo_init"])) {
    std::cout << i << std::endl;
  }


  glViewport(0, 0, 2, 2);
  glUniform1f(u_asp, aspect_ratio);

  glUniform2d(u_center, cx, cy);
  glUniform1d(u_scale, cur_scale);
  std::cout << std::setprecision(5) << cx << " " << cy
  << " "  << cur_scale << std::endl;
  glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &Projection[0][0]);

  glBindVertexArray(VertexArrayID);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);

  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);

  GLfloat pixels[2*2*2];
  glReadPixels(0, 0, 2, 2, GL_RG, GL_FLOAT, pixels);

  for (int i = 0; i < sizeof(pixels)/sizeof(GLfloat); i ++) {
    std::cout << pixels[i] << " ";
  }
  std::cout << std::endl;



}

Mandlebrot::~Mandlebrot(void) {
  glDeleteBuffers(1, &vertexbuffer);
  glDeleteBuffers(1, &uvbuffer);
  glDeleteFramebuffers(1, &handles["init"]);
  glDeleteProgram(init_prog);
  glDeleteProgram(show_prog);
  glDeleteProgram(kern_prog);
  glDeleteTextures(1, &TextureLoc);
  glDeleteVertexArrays(1, &VertexArrayID);
}
