#pragma once

#include <glad/glad.h>
#include <string>
#include <vector>

class WindowSurfaceConfig;

struct SurfaceData {
  std::string function;
  bool show;
  std::vector<float> vertices;
  std::vector<float> rgb;
  GLuint vao;
  GLuint vbo;
  GLuint ebo; // shared between all surfaces
  unsigned int ind_size;
  WindowSurfaceConfig* window_surface_config; // reference to respective window surface config
};
