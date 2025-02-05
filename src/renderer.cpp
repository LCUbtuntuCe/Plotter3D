#include "../include/renderer.hpp"
#include "../include/glm/glm.hpp"
#include "../include/glm/gtc/matrix_transform.hpp"
#include "../include/glm/gtc/type_ptr.hpp"
#include <GL/gl.h>


CanvasGL::CanvasGL(wxPanel* parent, int* args)
  : wxGLCanvas(parent, wxID_ANY, args, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE) {
  m_context = new wxGLContext(this);
  SetCurrent(*m_context);
  Bind(wxEVT_PAINT, &CanvasGL::render, this);
  Bind(wxEVT_SIZE, &CanvasGL::on_size, this);
}

CanvasGL::~CanvasGL() { delete m_context; }

void CanvasGL::on_size(wxSizeEvent &event) {
    int width, height;
    GetClientSize(&width, &height);
    glViewport(0, 0, width, height);
    Refresh();
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
void main() {
  gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

  /* ------------------- fragment shader ------------------- */

  const char *fragment_shader_source = R"(
#version 330 core
out vec4 FragColor;
void main() {
  FragColor = vec4(1.0f, 1.0f, 0.1f, 1.0f);
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

  glLineWidth(2);
  glPointSize(5);
  
}


void CanvasGL::render(wxPaintEvent& event) {

  if (!gl_has_been_init) {
    init_gl();
    gl_has_been_init = true;
  }
  
  SetCurrent(*m_context);
  wxPaintDC dc(this);

  glClearColor(0.5f, 0.6f, 0.2f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  glUseProgram(shader_program);

  glBindVertexArray(VAO);

  glm::mat4 view = glm::mat4(1.0f);
  //view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
  view = glm::lookAt(camera_pos, glm::vec3(0.0f, 0.0f, 0.0f), camera_up);
  GLuint locView = glGetUniformLocation(shader_program, "view");
  glUniformMatrix4fv(locView, 1, GL_FALSE, glm::value_ptr(view));

  int width, height;
  GetClientSize(&width, &height);
  float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
  glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 300.0f);
  GLuint locProjection = glGetUniformLocation(shader_program, "projection");
  glUniformMatrix4fv(locProjection, 1, GL_FALSE, glm::value_ptr(projection));

  glm::mat4 model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
  GLuint locModel = glGetUniformLocation(shader_program, "model");
  glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));

  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glDrawArrays(GL_TRIANGLES, 0, 36);

  SwapBuffers();

}
