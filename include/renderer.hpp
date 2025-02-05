#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif

#include <GL/glew.h>
// #include <GL/gl.h>
#include <GL/glext.h>
// #include <GL/glu.h>
#include <wx/wx.h>
#include <wx/glcanvas.h>

#include "../include/glm/glm.hpp"
#include "../include/glm/gtc/matrix_transform.hpp"
#include "../include/glm/gtc/type_ptr.hpp"

class CanvasGL : public wxGLCanvas {
  wxGLContext* m_context;
  GLuint shader_program;
  GLuint VBO, VAO;
  glm::vec3 camera_pos = glm::vec3(0.0f, 0.0f, -5.0f);
  glm::vec3 camera_front = glm::vec3(0.0f, 0.0f, -1.0f);
  glm::vec3 camera_up = glm::vec3(0.0f, 1.0f, 0.0f);
  float yaw = -90.0f;
  float pitch = 0.0f;
  float lastX = 300.0f / 2.0;
  float lastY = 300.0f / 2.0;
  float fov = 90.0f;
  bool gl_has_been_init = false;
public:
  CanvasGL(wxPanel* parent, int* args);
  virtual ~CanvasGL();
  void init_gl(void);
  void on_size(wxSizeEvent& event);
  void render(wxPaintEvent& event);
  void mouse_motion(wxMouseEvent& event);
  void mouse_left_down(wxMouseEvent& event);
  void mouse_left_up(wxMouseEvent& event);
  void mouse_right_down(wxMouseEvent& event);
  void mouse_right_up(wxMouseEvent& event);
};
