#include <map>
#include <string>
#include <iostream>
#include <ft2build.h>
#include FT_FREETYPE_H

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "text.hpp"


TextGL::TextGL() {};

TextGL::TextGL(const char * fontname, int size, glm::vec3 color) {
  FT_Library ft;
  FT_Face face;
  std::cout << "setting stuff up!\n";
  if (FT_Init_FreeType(&ft))
    std::cout << "ERROR: could not init freetype\n";
  if (FT_New_Face(ft, "/usr/share/fonts/truetype/droid/DroidSansMono.ttf", 0, &face))
    std::cout << "Error: could not init face\n";
  FT_Set_Pixel_Sizes(face, 0, size);

  textProg = LoadShaders("./text.vert", "./text.frag");

  // render characters to a texture.

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction

  for (GLubyte c = 0; c < 128; c++) {
    // Load character glyph
    if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
        std::cout << "ERROR::FREETYTPE: Failed to load Glyph\n";
        continue;
    }
    // Generate texture
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RED,
        face->glyph->bitmap.width,
        face->glyph->bitmap.rows,
        0,
        GL_RED,
        GL_UNSIGNED_BYTE,
        face->glyph->bitmap.buffer
    );
    // Set texture options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // Now store character for later use

    Character character = {
        .TextureID = texture,
        .Size = glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
        .Bearing = glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
        .Advance = GLuint(face->glyph->advance.x)
    };
    Characters.insert(std::pair<GLchar, Character>(c, character));
  }

  line_height = face->max_advance_height / 64;

  glBindTexture(GL_TEXTURE_2D, 0);

  FT_Done_Face(face);
  FT_Done_FreeType(ft);

  this->color = color;
  this->initOK = true;
}

void TextGL::print(std::string text, int x, int y, int screen_width, int screen_height) {
  auto x_start = x;  // auto y_start = y;
  if (!initOK) {std::cout << "error\n" ; return;}

  glUseProgram(textProg);

  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glActiveTexture(GL_TEXTURE0);
  glBindVertexArray(VAO);

  float scale = 1;
  auto textColor = glGetUniformLocation(textProg, "textColor");
  glUniform3f(textColor, color.x, color.y, color.z);

  glm::mat4 projection = glm::ortho(0.0f, float(screen_width), 0.0f, float(screen_height));

  auto projMatrix = glGetUniformLocation(textProg, "projection");
  glUniformMatrix4fv(projMatrix, 1, GL_FALSE, &projection[0][0]);

  std::string::const_iterator c;

  for (c = text.begin(); c!= text.end(); ++c) {
    if (*c == '\r') {
      x = x_start;
    }
    if (*c == '\n') {
      x = x_start;
      y -= line_height;
    }
    if (*c < 32) continue;
    Character ch = Characters[*c];

    GLfloat xpos = x +  ch.Bearing.x + scale;
    GLfloat ypos = y - (ch.Size.y - ch.Bearing.y ) * scale;
    GLfloat w = ch.Size.x * scale;
    GLfloat h = ch.Size.y * scale;
    // Update VBO for each character
    GLfloat vertices[6][4] = {
        { xpos,     ypos + h,   0.0, 0.0 },
        { xpos,     ypos,       0.0, 1.0 },
        { xpos + w, ypos,       1.0, 1.0 },

        { xpos,     ypos + h,   0.0, 0.0 },
        { xpos + w, ypos,       1.0, 1.0 },
        { xpos + w, ypos + h,   1.0, 0.0 }
    };
    // Render glyph texture over quad
    glBindTexture(GL_TEXTURE_2D, ch.TextureID);
    // Update content of VBO memory
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    // Render quad
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
    x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64)
    // std::cout << *c;
    glDisable(GL_BLEND);
  }
  // glDisableVertexAttribArray(0);
  glBindVertexArray(0);
  glBindTexture(GL_TEXTURE_2D, 0);
}