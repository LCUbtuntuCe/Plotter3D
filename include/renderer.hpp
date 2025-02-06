#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif

#include <GL/glew.h>
#include <GL/glext.h>
#include <wx/wx.h>
#include <wx/glcanvas.h>

#include "../include/properties.hpp"

#include "../include/glm/glm.hpp"
#include "../include/glm/gtc/matrix_transform.hpp"
#include "../include/glm/gtc/type_ptr.hpp"

class CanvasGL : public wxGLCanvas {
  Properties& props;
  wxGLContext* m_context;
  GLuint shader_program;
  GLuint VBO, VAO, VAO_AXIS, VBO_AXIS, VAO1, VBO1, EBO1;
  std::vector<float> vertices1;
  std::vector<unsigned int> indices;
  float radius = 5.0f;
  float theta = glm::radians(-90.0f);
  float phi = glm::radians(0.0f);
  float scale_translation = 0.04f;
  float scale_rotation = 0.01f;
  float fov = 90.0f;
  glm::vec3 camera_pos = glm::vec3(0.0f, 0.0f, -radius);
  glm::vec3 camera_front = glm::vec3(0.0f, 0.0f, -1.0f);
  glm::vec3 camera_up = glm::vec3(0.0f, 1.0f, 0.0f);
  int x_current, y_current;
  int x_last, y_last;
  bool gl_has_been_init = false;
  bool left_is_down = false;
  bool right_is_down = false;
public:
  CanvasGL(wxPanel* parent, int* args, Properties& properties);
  virtual ~CanvasGL();
  void init_gl(void);
  void on_size(wxSizeEvent& event);
  void render(wxPaintEvent& event);
  void on_mouse_motion(wxMouseEvent& event);
  void on_mouse_left_down(wxMouseEvent& event);
  void on_mouse_left_up(wxMouseEvent& event);
  void on_mouse_right_down(wxMouseEvent& event);
  void on_mouse_right_up(wxMouseEvent& event);
};
