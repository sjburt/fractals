#ifndef MAND_HPP
#define MAND_HPP

#include <map>
#include <string>
#include <GL/glew.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


class Mandlebrot {
 private:

  std::map<std::string, GLuint> handles;

  GLuint VertexArrayID;

  GLuint init_prog;
  GLuint kern_prog;
  GLuint show_prog;

  GLuint vertexbuffer;
  GLuint uvbuffer;

  GLuint zbuffer;

  GLuint TextureLoc;
  GLuint TextureID;
  GLuint MatrixID, iterID;

  GLuint u_iter, u_asp, u_center, u_scale;

  glm::mat4 Projection;
  const GLfloat g_vertex_buffer_data[4][4] = {
      {-1, -1, 0, 1}, {-1, 1, 0, 1}, {1, -1, 0, 1}, {1, 1, 0, 1}};

  GLfloat g_textcoords[4][2] = {
      {-1.0, -1.0}, {-1.0, 1.0}, {1.0, -1.0}, {1.0, 1.0}};



 public:
  Mandlebrot(void);
  void init(void);
  void render(const int iter, const float aspect_ratio, const double cx,
              const double cy, const double scale);
  ~Mandlebrot(void);
};

#endif  // MAND_HPP