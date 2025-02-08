#pragma once

#include <glad/glad.h>
#include <window_surface_config.hpp>

struct Surfaces {
  int id;
  //WindowSurfaceConfig* surface_config_window;
  GLuint VAO;            // to draw or update data
  unsigned int ind_num;  // number of indices in EBO
};
