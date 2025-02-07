#include "../include/renderer.hpp"
#include "../include/glm/glm.hpp"
#include "../include/glm/gtc/matrix_transform.hpp"
#include "../include/glm/gtc/type_ptr.hpp"
#include "../include/parser.hpp"
#include <GL/gl.h>
#include <vector>
#include <wx/event.h>


CanvasGL::CanvasGL(wxPanel* parent, int* args, Properties& properties)
  : wxGLCanvas(parent, wxID_ANY, args, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE),
    props(properties) {
  m_context = new wxGLContext(this);
  SetCurrent(*m_context);
  Bind(wxEVT_PAINT,      &CanvasGL::render, this);
  Bind(wxEVT_SIZE,       &CanvasGL::on_size, this);
  Bind(wxEVT_MOTION,     &CanvasGL::on_mouse_motion, this);
  Bind(wxEVT_LEFT_DOWN,  &CanvasGL::on_mouse_left_down, this);
  Bind(wxEVT_LEFT_UP,    &CanvasGL::on_mouse_left_up, this);
  Bind(wxEVT_RIGHT_DOWN, &CanvasGL::on_mouse_right_down, this);
  Bind(wxEVT_RIGHT_UP,   &CanvasGL::on_mouse_right_up, this);
}

CanvasGL::~CanvasGL() { delete m_context; }

void CanvasGL::on_size(wxSizeEvent &event) {
    int width, height;
    GetClientSize(&width, &height);
    glViewport(0, 0, width, height);
    Refresh();
}

void CanvasGL::on_mouse_motion(wxMouseEvent& event) {

  if (!HasCapture())
    return;

  x_last = x_current;
  y_last = y_current;
  wxPoint pos = event.GetPosition();
  x_current = pos.x;
  y_current = pos.y;

  if (left_is_down) {

    // calculate offset using coordinate differences
    float x_offset = (x_current - x_last) * scale_rotation;
    float y_offset = -(y_current - y_last) * scale_rotation;
    theta += x_offset;
    phi -= y_offset;

    // constrain phi to avoid flipping the camera
    phi = glm::clamp(phi, glm::radians(-89.0f), glm::radians(89.0f));

    // convert spherical coordinates to cartesian coordinates
    float x = radius * cos(phi) * cos(theta);
    float y = radius * sin(phi);
    float z = radius * cos(phi) * sin(theta);
    
    // update camera position
    camera_pos = glm::vec3(x, y, z);
    
  } else if (right_is_down) {

    if (props.perspective) {
      
      // calculate translation value
      float tval = (y_current - y_last) * scale_translation;

      // calculate direction vector from the camera position to the origin
      glm::vec3 direction_vector = glm::vec3(0.0f, 0.0f, 0.0f) - camera_pos;

      // calculate distance between the camera and the origin
      float distance = glm::length(direction_vector);

      // dont get too close to the origin
      if (distance < 2.0f && tval > 0) {
	event.Skip();
	return;
      }

      // update radius
      radius = distance;

      // normalize direction vector to get the unit direction
      glm::vec3 unit_direction = glm::normalize(direction_vector);

      // calculate translation vector
      glm::vec3 translation_vector = unit_direction * tval;

      // translate camera position
      camera_pos += translation_vector;
    } else {
      float offset = -(y_current - y_last) * scale_translation;
      ortho_size += offset;
    }
  }
  Refresh();
}

void CanvasGL::on_mouse_left_down(wxMouseEvent& event) {
  if (!HasCapture()) {
    CaptureMouse();
    x_current = x_last = event.GetX();
    y_current = y_last = event.GetY();
    left_is_down = true;
  }
}
void CanvasGL::on_mouse_left_up(wxMouseEvent& event) {
  if (HasCapture()) {
    ReleaseMouse();
    left_is_down = false;
  }
}
void CanvasGL::on_mouse_right_down(wxMouseEvent& event) {
  if (!HasCapture()) {
    CaptureMouse();
    x_current = x_last = event.GetX();
    y_current = y_last = event.GetY();
    right_is_down = true;
  }
}
void CanvasGL::on_mouse_right_up(wxMouseEvent& event) {
  if (HasCapture()) {
    ReleaseMouse();
    right_is_down = false;
  }
}

void CanvasGL::add_surface(const std::string& function, std::vector<float>& rgb_color) {
  parser p;
  std::vector<float> vert;
  std::vector<unsigned int> ind;
  int estimatedSize = (props.grid_size + 1) * (props.grid_size + 1) * 3;
  vert.reserve(estimatedSize);
  ind.reserve(estimatedSize / 3);
  // convert function string to char*
  char* expression = new char[function.size() + 1];
  std::strcpy(expression, function.c_str());

  float start = -props.grid_size / 2.0f;
  int num_vertices_per_axis = props.divisions + 1;
  double step = props.grid_size / props.divisions;

  // vertices
  for (int i = 0; i < num_vertices_per_axis; i++) {
    for (int j = 0; j < num_vertices_per_axis; j++) {
      float x = start + i * step;
      float y = start + j * step;

      p.set_xy(static_cast<double>(x), static_cast<double>(y));
      double z = p.eval_expr(expression);

      vert.push_back(x);
      vert.push_back(static_cast<float>(z));
      vert.push_back(y);

      vert.push_back(rgb_color[0]);
      vert.push_back(rgb_color[1]);
      vert.push_back(rgb_color[2]);
    }
  }
  // indices
  for (int i = 0; i < props.divisions; i++) {
    for (int j = 0; j < props.divisions; j++) {
      int row1 = i * num_vertices_per_axis;
      int row2 = (i + 1) * num_vertices_per_axis;
      
      ind.push_back(row1 + j);
      ind.push_back(row2 + j);
      ind.push_back(row1 + j + 1);

      ind.push_back(row1 + j + 1);
      ind.push_back(row2 + j);
      ind.push_back(row2 + j + 1);
    }
  }

  GLuint vao, vbo, ebo;
  // create array and buffers
  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);
  glGenBuffers(1, &ebo);
  // bind vao
  glBindVertexArray(vao);
  // pass data to vbo buffer
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, vert.size() * sizeof(float), vert.data(), GL_STATIC_DRAW);
  // pass data to ebo buffer
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, ind.size() * sizeof(unsigned int), ind.data(), GL_STATIC_DRAW);
  // set location and data format 
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
  // unbind
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
  // add to vector
  Surface3D surf = {function, true, vao, (int)ind.size()};
  surfaces.push_back(surf);
}


void CanvasGL::init_gl(void) {

  SetCurrent(*m_context);
  
  glewExperimental = GL_TRUE;
  glewInit();
  SetBackgroundStyle(wxBG_STYLE_CUSTOM);

  /* -------------------- vertex shader -------------------- */

  const char *shader_source_vertex = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
out vec4 input_color;
void main() {
  gl_Position = projection * view * model * vec4(aPos, 1.0);
  input_color = vec4(aColor, 1.0);
}
)";

  /* ------------------- fragment shaders ------------------- */

  const char *shader_source_fragment_surface = R"(
#version 330 core
in vec4 input_color;
out vec4 FragColor;
void main() {
  // FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
  FragColor = input_color;
}
)";

    const char *shader_source_fragment_mesh = R"(
#version 330 core
in vec4 input_color;
out vec4 FragColor;
void main() {
  FragColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
}
)";

  /* -------------------- shader surface -------------------- */

  GLuint shader_vertex = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(shader_vertex, 1, &shader_source_vertex, NULL);
  glCompileShader(shader_vertex);

  GLuint shader_fragment_surface = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(shader_fragment_surface, 1, &shader_source_fragment_surface, NULL);
  glCompileShader(shader_fragment_surface);

  shader_surface = glCreateProgram();
  glAttachShader(shader_surface, shader_vertex);
  glAttachShader(shader_surface, shader_fragment_surface);
  glLinkProgram(shader_surface);

  /* --------------------- mesh shader --------------------- */

  GLuint shader_fragment_mesh = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(shader_fragment_mesh, 1, &shader_source_fragment_mesh, NULL);
  glCompileShader(shader_fragment_mesh);

  shader_mesh = glCreateProgram();
  glAttachShader(shader_mesh, shader_vertex);
  glAttachShader(shader_mesh, shader_fragment_mesh);
  glLinkProgram(shader_mesh);

  glDeleteShader(shader_vertex);
  glDeleteShader(shader_fragment_surface);
  glDeleteShader(shader_fragment_mesh);

  /* ------------------------- axis ------------------------- */

  glGenVertexArrays(1, &VAO_AXIS);
  glGenBuffers(1, &VBO_AXIS);

  glBindVertexArray(VAO_AXIS);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_AXIS);
  float s = 10.0f;
  float axis[] = {
    s, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
    -s, 0.0f, 0.0f, 0.2f, 0.0f, 0.0f,
    0.0f, s, 0.0f, 0.0f, 0.0f, 1.0f,
    0.0f, -s, 0.0f, 0.0f, 0.0f, 0.2f,
    0.0f, 0.0f, s, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, -s, 0.0f, 0.2f, 0.0f
  };
  glBufferData(GL_ARRAY_BUFFER, sizeof(axis), axis, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  /* ------------------------- vars ------------------------- */

  glLineWidth(2);
  glPointSize(10);
  glEnable(GL_DEPTH_TEST);

  std::vector<float> color1{0.3f, 0.4f, 0.2f};
  std::vector<float> color2{0.6f, 0.2f, 0.2f};
  add_surface("x * y", color1);
  add_surface("x * x", color2);
}

void CanvasGL::render(wxPaintEvent& event) {

  if (!gl_has_been_init) {
    init_gl();
    gl_has_been_init = true;
  }
  
  SetCurrent(*m_context);
  wxPaintDC dc(this);

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

  /* ------------------- transformations ------------------- */

  glm::mat4 view = glm::mat4(1.0f);
  view = glm::lookAt(camera_pos, glm::vec3(0.0f, 0.0f, 0.0f), camera_up);

  int width, height;
  GetClientSize(&width, &height);
  float aspectRatio = static_cast<float>(width) / static_cast<float>(height);

  glm::mat4 projection;
  if (props.perspective) {
    projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 5000.0f);
  } else {
    float left = -ortho_size * aspectRatio;
    float right = ortho_size * aspectRatio;
    float bottom = -ortho_size;
    float top = ortho_size;
    projection = glm::ortho<float>(left, right, bottom, top, 0.1f, 5000.0f);
  }

  glm::mat4 model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));

  /* ----------------------- surfaces ----------------------- */

  glUseProgram(shader_surface);
  glEnable(GL_DEPTH_TEST);

  GLuint locView = glGetUniformLocation(shader_surface, "view");
  glUniformMatrix4fv(locView, 1, GL_FALSE, glm::value_ptr(view));
  GLuint locProjection = glGetUniformLocation(shader_surface, "projection");
  glUniformMatrix4fv(locProjection, 1, GL_FALSE, glm::value_ptr(projection));
  GLuint locModel = glGetUniformLocation(shader_surface, "model");
  glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));

  for (Surface3D i : surfaces) {
    glBindVertexArray(i.vao);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawElements(GL_TRIANGLES, i.ind_size, GL_UNSIGNED_INT, 0);
  }

  /* ------------------------- mesh ------------------------- */

  if (props.show_mesh) {
    glUseProgram(shader_mesh);
    glEnable(GL_DEPTH_TEST);

    locView = glGetUniformLocation(shader_mesh, "view");
    glUniformMatrix4fv(locView, 1, GL_FALSE, glm::value_ptr(view));
    locProjection = glGetUniformLocation(shader_mesh, "projection");
    glUniformMatrix4fv(locProjection, 1, GL_FALSE, glm::value_ptr(projection));
    locModel = glGetUniformLocation(shader_mesh, "model");
    glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));

    glLineWidth(4);
    for (Surface3D i : surfaces) {
      glBindVertexArray(i.vao);
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      glDrawElements(GL_TRIANGLES, i.ind_size, GL_UNSIGNED_INT, 0);
    }
  }

  /* ------------------------- axes ------------------------- */

  if (props.show_axes) {
    glUseProgram(shader_surface);
    glDisable(GL_DEPTH_TEST);
    glLineWidth(5);
    glBindVertexArray(VAO_AXIS);
    glDrawArrays(GL_LINES, 0, 6);
  }

  SwapBuffers();

}
