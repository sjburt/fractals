#ifndef MAND_HPP
#define MAND_HPP

#include <GL/glew.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


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

#endif  // MAND_HPP