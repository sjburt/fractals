#ifndef TEXT_HPP
#define TEXT_HPP

#include <memory>
#include <map>
#include <string>

#include <glfw3.h>
#include <glm/glm.hpp>

#include "../common/shader.hpp"

class TextGL {

public:
  TextGL(const char * fontname, int size, glm::vec3 color);
  TextGL(void);
  void print(GLFWwindow* window, std::string text, int x, int y);
private:
  struct Character {
    GLuint     TextureID;  // ID handle of the glyph texture
    glm::ivec2 Size;       // Size of glyph
    glm::ivec2 Bearing;    // Offset from baseline to left/top of glyph
    GLuint     Advance;    // Offset to advance to next glyph
  };
  bool initOK = false;
  std::map<GLchar, Character> Characters;
  GLuint textProg;
  GLuint VAO, VBO;
  glm::vec3 color;
};

#endif