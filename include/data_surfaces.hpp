#pragma once

#include <glad/glad.h>
#include <string>
#include <vector>

struct SurfaceData {
  std::string function;
  bool show;
  std::vector<float> vertices;
  std::vector<float> rgb;
  GLuint vao;
  GLuint vbo;
  unsigned int ind_size;
  // GLuint& ebo; // shared between all surfaces
};
