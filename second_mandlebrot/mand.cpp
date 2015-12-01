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

// Create and compile our GLSL program from the shaders
  init_prog = LoadShaders("passthrough.vert", "init.frag");
  if (!init_prog) exit(-1);

  kern_prog = LoadShaders("passthrough.vert", "mand_kern.frag");
  if (!kern_prog) exit(-1);

  show_prog = LoadShaders("passthrough.vert", "mand_show.frag");
  if (!show_prog) exit(-1);


  // Make a square.
  glGenVertexArrays(1, &handles["init_VAO"]);
  glBindVertexArray(handles["init_VAO"]);

  glGenBuffers(1, &handles["init_VBO"]);
  glBindBuffer(GL_ARRAY_BUFFER, handles["init_VBO"]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data),
               g_vertex_buffer_data, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, NULL);
  glEnableVertexAttribArray(0);
  glGenBuffers(1, &handles["init_uv_VBO"]);
  glBindBuffer(GL_ARRAY_BUFFER, handles["init_uv_VBO"]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_textcoords), g_textcoords,
               GL_DYNAMIC_DRAW);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
  glEnableVertexAttribArray(1);

  glBindVertexArray(0);

  glDeleteBuffers(1, &handles["kern_VBO"]);
  glDeleteBuffers(1, &handles["kern_uv_VBO"]);


  glGenVertexArrays(1, &handles["kern_VAO"]);
  glBindVertexArray(handles["kern_VAO"]);

  glGenBuffers(1, &handles["kern_VBO"]);
  glBindBuffer(GL_ARRAY_BUFFER, handles["kern_VBO"]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data),
               g_vertex_buffer_data, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, NULL);
  glEnableVertexAttribArray(0);
  glGenBuffers(1, &handles["kern_uv_VBO"]);
  glBindBuffer(GL_ARRAY_BUFFER, handles["kern_uv_VBO"]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_textcoords), g_textcoords,
               GL_DYNAMIC_DRAW);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
  glEnableVertexAttribArray(1);

  glBindVertexArray(0);

  glDeleteBuffers(1, &handles["kern_VBO"]);
  glDeleteBuffers(1, &handles["kern_uv_VBO"]);




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

  handles["coordinate"] = glGetUniformLocation(kern_prog, "location");

  handles["iter"] = glGetUniformLocationARB(kern_prog, "iter");
  handles["aspect_ratio"] = glGetUniformLocationARB(init_prog, "aspect");

  handles["scale"] = glGetUniformLocation(init_prog, "scale");
  handles["center"] = glGetUniformLocationARB(init_prog, "center");

  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // generate a framebuffer to render to

  glGenFramebuffers(1, &handles["fbo_init"]);
  glBindFramebuffer(GL_FRAMEBUFFER, handles["fbo_init"]);

  glGenTextures(1, &handles["tex_init"]);
  glBindTexture(GL_TEXTURE_2D, handles["tex_init"]);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, 1024, 768, 0, GL_RG, GL_UNSIGNED_BYTE, 0);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
    handles["tex_init"], 0);

  // Set the list of draw buffers.
  GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
  glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

  // Always check that our framebuffer is ok
  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cout << "framebuffer was not properly initialized. \n";
    exit(1);
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void Mandlebrot::setActiveFramebuffer(std::string buffer) {
  glBindFramebuffer(GL_FRAMEBUFFER, handles[buffer]);
}

void Mandlebrot::setActiveFramebuffer(int buffer) {
  glBindFramebuffer(GL_FRAMEBUFFER, buffer);
}

void Mandlebrot::reinit(int width, int height, const float aspect_ratio,
                        const double cx, const double cy,
                        const double cur_scale) {
  // GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
  // glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glBindTexture(GL_TEXTURE_2D, handles["tex_init"]);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, width, height, 0, GL_RG, GL_UNSIGNED_BYTE, 0);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glUseProgram(init_prog);

  glBindFramebuffer(GL_FRAMEBUFFER, handles["fbo_init"]);

  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cout << "framebuffer was not properly initialized. \n";
    exit(1);
  }

  glUniform1f(handles["aspect_ratio"], aspect_ratio);
  glUniform2d(handles["center"], cx, cy);
  glUniform1d(handles["scale"], cur_scale);

  glBindVertexArray(handles["init_VAO"]);

  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void Mandlebrot::render(const int iter) {


  // render to the screen.
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  GLenum DrawBuffers[1] = {GL_BACK_LEFT};
  glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glUseProgram(kern_prog);
  glUniform1i(handles["iter"], iter);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, handles["tex_init"]);
  // glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, width, height, 0, GL_RG, GL_UNSIGNED_BYTE, 0);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


  glUniform1i(handles["coordinate"], 0);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_1D, TextureID);
  glUniform1i(TextureLoc, 1);

  glBindVertexArray(handles["kern_VAO"]);

  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glBindVertexArray(0);


}

Mandlebrot::~Mandlebrot(void) {
  glDeleteBuffers(1, &handles["vertex_buffer_init"]);
  glDeleteBuffers(1, &handles["uv_loc_buffer"]);
  glDeleteFramebuffers(1, &handles["init"]);
  glDeleteProgram(init_prog);
  glDeleteProgram(show_prog);
  glDeleteProgram(kern_prog);
  glDeleteTextures(1, &TextureLoc);
  glDeleteVertexArrays(1, &handles["init_VAO"]);
}
