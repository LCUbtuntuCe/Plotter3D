#include "../include/renderer.hpp"
#include "../include/glm/glm.hpp"
#include "../include/glm/gtc/matrix_transform.hpp"
#include "../include/glm/gtc/type_ptr.hpp"
#include <GL/gl.h>
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

const int resolution = 50;
const float size = 5.0f;
void generate_paraboloid(std::vector<float>& vertices, std::vector<unsigned int>& indices) {
    for (int i = 0; i <= resolution; ++i) {
        for (int j = 0; j <= resolution; ++j) {
            float x = size * ((float)i / resolution - 0.5f);
            float y = size * ((float)j / resolution - 0.5f);
            float z = x * x + y * y;
            vertices.push_back(x);
	    vertices.push_back(z);
            vertices.push_back(y);
        }
    }
    for (int i = 0; i < resolution; ++i) {
        for (int j = 0; j < resolution; ++j) {
            int row1 = i * (resolution + 1);
            int row2 = (i + 1) * (resolution + 1);

            indices.push_back(row1 + j);
            indices.push_back(row2 + j);
            indices.push_back(row1 + j + 1);

            indices.push_back(row1 + j + 1);
            indices.push_back(row2 + j);
            indices.push_back(row2 + j + 1);
        }
    }
}

void CanvasGL::init_gl(void) {

  SetCurrent(*m_context);
  
  glewExperimental = GL_TRUE;
  glewInit();
  SetBackgroundStyle(wxBG_STYLE_CUSTOM);

  /* -------------------- vertex shader -------------------- */

  const char *vertex_shader_source = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
out vec4 input_color;
void main() {
  gl_Position = projection * view * model * vec4(aPos, 1.0);
  input_color = vec4(aPos, 1.0);
}
)";

  /* ------------------- fragment shader ------------------- */

  const char *fragment_shader_source = R"(
#version 330 core
in vec4 input_color;
out vec4 FragColor;
void main() {
  //FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
  FragColor = input_color;
}
)";

  GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
  glCompileShader(vertex_shader);

  GLint success;
  GLchar infoLog[512];
  glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertex_shader, 512, NULL, infoLog);
    std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
  }

  GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
  glCompileShader(fragment_shader);

  glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fragment_shader, 512, NULL, infoLog);
    std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
  }

  shader_program = glCreateProgram();
  glAttachShader(shader_program, vertex_shader);
  glAttachShader(shader_program, fragment_shader);
  glLinkProgram(shader_program);

  glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(shader_program, 512, NULL, infoLog);
    std::cerr << "ERROR::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
  }

  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);

  float vertices[] = {
    -0.5f, -0.5f, -0.5f,
    0.5f, -0.5f, -0.5f,
    0.5f,  0.5f, -0.5f,
    0.5f,  0.5f, -0.5f,
    -0.5f,  0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,

    -0.5f, -0.5f,  0.5f,
    0.5f, -0.5f,  0.5f,
    0.5f,  0.5f,  0.5f,
    0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,
    -0.5f, -0.5f,  0.5f,

    -0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,

    0.5f,  0.5f,  0.5f,
    0.5f,  0.5f, -0.5f,
    0.5f, -0.5f, -0.5f,
    0.5f, -0.5f, -0.5f,
    0.5f, -0.5f,  0.5f,
    0.5f,  0.5f,  0.5f,

    -0.5f, -0.5f, -0.5f,
    0.5f, -0.5f, -0.5f,
    0.5f, -0.5f,  0.5f,
    0.5f, -0.5f,  0.5f,
    -0.5f, -0.5f,  0.5f,
    -0.5f, -0.5f, -0.5f,

    -0.5f,  0.5f, -0.5f,
    0.5f,  0.5f, -0.5f,
    0.5f,  0.5f,  0.5f,
    0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f, -0.5f,
  };
  
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, 0); 
  glBindVertexArray(0);

  /* ------------------------- axis ------------------------- */

  glGenVertexArrays(1, &VAO_AXIS);
  glGenBuffers(1, &VBO_AXIS);

  glBindVertexArray(VAO_AXIS);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_AXIS);
  float axis[] = {
    5.0f, 0.0f, 0.0f,
    -5.0f, 0.0f, 0.0f,
    0.0f, 5.0f, 0.0f,
    0.0f, -5.0f, 0.0f,
    0.0f, 0.0f, 5.0f,
    0.0f, 0.0f, -5.0f
  };
  glBufferData(GL_ARRAY_BUFFER, sizeof(axis), axis, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  /* ------------------------- test ------------------------- */

  generate_paraboloid(vertices1, indices);
  glGenVertexArrays(1, &VAO1);
  glGenBuffers(1, &VBO1);
  glGenBuffers(1, &EBO1);
  glBindVertexArray(VAO1);
  glBindBuffer(GL_ARRAY_BUFFER, VBO1);
  glBufferData(GL_ARRAY_BUFFER, vertices1.size() * sizeof(float), vertices1.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO1);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  /* ------------------------- vars ------------------------- */

  glLineWidth(2);
  glPointSize(10);
  glEnable(GL_DEPTH_TEST);

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

  glUseProgram(shader_program);

  glm::mat4 view = glm::mat4(1.0f);
  view = glm::lookAt(camera_pos, glm::vec3(0.0f, 0.0f, 0.0f), camera_up);
  GLuint locView = glGetUniformLocation(shader_program, "view");
  glUniformMatrix4fv(locView, 1, GL_FALSE, glm::value_ptr(view));

  int width, height;
  GetClientSize(&width, &height);
  float aspectRatio = static_cast<float>(width) / static_cast<float>(height);

  glm::mat4 projection;
  if (props.perspective) {
    projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 300.0f);
  } else {
    float left = -ortho_size * aspectRatio;
    float right = ortho_size * aspectRatio;
    float bottom = -ortho_size;
    float top = ortho_size;
    projection = glm::ortho<float>(left, right, bottom, top, 0.1f, 300.0f);
  }
				    
  GLuint locProjection = glGetUniformLocation(shader_program, "projection");
  glUniformMatrix4fv(locProjection, 1, GL_FALSE, glm::value_ptr(projection));

  glm::mat4 model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
  GLuint locModel = glGetUniformLocation(shader_program, "model");
  glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));

  // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  // glBindVertexArray(VAO);
  // glDrawArrays(GL_TRIANGLES, 0, 36);

  if (props.show_axes) {
    glBindVertexArray(VAO_AXIS);
    glDrawArrays(GL_LINES, 0, 6);
  }

  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glBindVertexArray(VAO1);
  glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  // glDrawArrays(GL_TRIANGLES, 0, 36);


  

  SwapBuffers();

}
