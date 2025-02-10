#pragma once

#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <data_properties.hpp>
#include <wx/wx.h>
#include <wx/glcanvas.h>
#include <string>
#include <map>
#include <data_surfaces.hpp>
#include <window_surface_config.hpp>

class CanvasGL : public wxGLCanvas {
  wxGLContext* m_context;
  GLuint shader_surface, shader_mesh;
  GLuint VAO_AXIS, VBO_AXIS;
  GLuint EBO;
  bool ebo_has_been_init;
  float fov            = 60.0f;
  float near_plane     = 0.05;
  float far_plane      = 5000;
  float ortho_size     = 5.0f;
  float radius = 5.0f;
  float theta  = glm::radians(-90.0f);
  float phi    = glm::radians(0.0f);
  float scale_translation = 0.5f;
  float scale_rotation    = 0.01f;
  glm::vec3 camera_pos = glm::vec3(0.0f, 0.0f, -radius);
  bool gl_has_been_init = false;
  bool left_is_down     = false;
  bool right_is_down    = false;
  int x_current, y_current, x_last, y_last;
  Properties& props;
  std::map<unsigned int, SurfaceData>& surfaces_data;
  // std::vector<SurfaceRender> surfaces;
  std::vector<float> vertices1;
  std::vector<unsigned int> indices;
  void add_surface(const std::string& function, std::vector<float>& rgb_color);
public:
  CanvasGL(wxPanel* parent, int* args, Properties& properties, std::map<unsigned int, SurfaceData>& surfaces_data);
  virtual ~CanvasGL();
  void init_gl(void);
  void on_size(wxSizeEvent& event);
  void render(wxPaintEvent& event);
  void on_mouse_motion(wxMouseEvent& event);
  void on_mouse_left_down(wxMouseEvent& event);
  void on_mouse_left_up(wxMouseEvent& event);
  void on_mouse_right_down(wxMouseEvent& event);
  void on_mouse_right_up(wxMouseEvent& event);
  void ebo_update();
};
