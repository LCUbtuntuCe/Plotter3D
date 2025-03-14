#include <glad/glad.h>
#include <renderer.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <parser.hpp>
#include <vector>
#include <wx/event.h>
#include <window_surface_config.hpp>

// 构造函数
/* 这里所用的wxGLCanvas类的构造函数应该是:
	wxGLCanvas::wxGLCanvas(	
		wxWindow* parent,
		wxWindowID id = wxID_ANY,
		const int* attribList = NULL,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = 0,
		const wxString& name = "GLCanvas",
		const wxPalette& palette = wxNullPalette 
	)
 此构造函数目前仅出于兼容性原因而保留。
*/
CanvasGL::CanvasGL(wxPanel* parent, int* args, Properties& properties, std::map<unsigned int, SurfaceData>& surfaces_data)
  : wxGLCanvas(parent, wxID_ANY, args, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE),
    props(properties),
    surfaces_data(surfaces_data)
{
	/* 
	wxGLContext的实例既承载了OpenGL状态机的实时运行状态，
	也维系着OpenGL图形接口与操作系统底层（如窗口系统、硬件驱动）之间的核心交互通道。
	*/
	m_context = new wxGLContext(this);

	// Bind函数用于将给定的函数、方法和事件绑定:

	// wxEVT_PAINT是一个事件宏，用于标识绘图事件
	// 绘图事件包括: 窗口首次创建时; 窗口被最小化然后恢复; 窗口被覆盖后再次显示; 代码主动调用Refresh()或Update()
	Bind(wxEVT_PAINT,      &CanvasGL::render, this);

	// 当窗口的尺寸发生变化（例如用户调整窗口大小、最大化、最小化等）时，就会触发wxEVT_SIZE事件。
	Bind(wxEVT_SIZE,       &CanvasGL::on_size, this);

	// 当鼠标在窗口内部移动时，就会触发wxEVT_MOTION事件。
	Bind(wxEVT_MOTION,     &CanvasGL::on_mouse_motion, this);

	// 按下鼠标左键
	Bind(wxEVT_LEFT_DOWN,  &CanvasGL::on_mouse_left_down, this);

	// 松开鼠标左键
	Bind(wxEVT_LEFT_UP,    &CanvasGL::on_mouse_left_up, this);

	// 按下鼠标右键
	Bind(wxEVT_RIGHT_DOWN, &CanvasGL::on_mouse_right_down, this);

	// 松开鼠标右键
	Bind(wxEVT_RIGHT_UP,   &CanvasGL::on_mouse_right_up, this);
}

// 析构函数: 销毁OpenGL上下文
CanvasGL::~CanvasGL() { delete m_context; }

// 初始化OpenGL
void CanvasGL::init_gl(void) 
{
	// 由OpenGL渲染上下文m_context表示的状态成为当前状态，m_context将被用于后续所有的OpenGL调用
	SetCurrent(*m_context);

	// 该函数属于GLAD库，在创建了OpenGL上下文之后动态加载当前上下文中可用的OpenGL函数地址
	if(!gladLoadGL())
	{
		std::cerr << "Failed to initialize GLAD" << std::endl;
		return;
	}

	// 设置窗口的背景风格，
	/* wxBG_STYLE_CUSTOM表示控件的背景绘制由控件自身负责，而不是由系统自动擦除或绘制。
	   这通常用于需要完全自定义绘制效果的控件，开发者需要在相应的绘制事件(如EVT_PAINT)中处理背景的绘制工作
    	*/
	SetBackgroundStyle(wxBG_STYLE_CUSTOM);

	// ------------------------------------------------------------
	// vertex shader
  	// ------------------------------------------------------------
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

	// ------------------------------------------------------------
	// fragment shaders
	// ------------------------------------------------------------

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

	// ------------------------------------------------------------
	// surface shader
	// ------------------------------------------------------------

	// glCreateShader用于创建一个新的着色器对象，失败了就返回0
	// GL_VERTEX_SHADER类型的着色器是一种旨在在可编程顶点处理器上运行的着色器。
	GLuint shader_vertex = glCreateShader(GL_VERTEX_SHADER);
	// glShaderSource函数将着色器对象中的源代码设置为由string参数指定的字符串数组中的源代码。之前存储在着色器对象中的任何源代码会被完全替换。
	// 数组中的字符串数量由count参数指定, 这里就一个字符串。
	glShaderSource(shader_vertex, 1, &shader_source_vertex, NULL);
	// 指出要被编译的着色器对象
	glCompileShader(shader_vertex);

	// 类型为GL_FRAGMENT_SHADER的着色器是一种专为在可编程片段处理器上运行而设计的着色器。
	GLuint shader_fragment_surface = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(shader_fragment_surface, 1, &shader_source_fragment_surface, NULL);
	// 指出要被编译的着色器对象
	glCompileShader(shader_fragment_surface);

	// glCreateProgram函数创建一个空的程序对象，并返回一个非零值作为其引用标识符。程序对象是一个可以附加着色器对象的容器。
	shader_surface = glCreateProgram();
	// 附加着色器对象
	glAttachShader(shader_surface, shader_vertex);
	// 附加着色器对象
	glAttachShader(shader_surface, shader_fragment_surface);
	// 指明要被链接的程序对象句柄
	glLinkProgram(shader_surface);

	// ------------------------------------------------------------
	// mesh shader
	// ------------------------------------------------------------

	// 类型为GL_FRAGMENT_SHADER的着色器是一种专为在可编程片段处理器上运行而设计的着色器
	GLuint shader_fragment_mesh = glCreateShader(GL_FRAGMENT_SHADER);
	// glShaderSource函数将着色器对象中的源代码设置为由string参数指定的字符串数组中的源代码。之前存储在着色器对象中的任何源代码会被完全替换。
	// 数组中的字符串数量由count参数指定, 这里就一个字符串。
	glShaderSource(shader_fragment_mesh, 1, &shader_source_fragment_mesh, NULL);
	// 指出要被编译的着色器对象
	glCompileShader(shader_fragment_mesh);

	// glCreateProgram函数创建一个空的程序对象，并返回一个非零值作为其引用标识符。程序对象是一个可以附加着色器对象的容器。
	shader_mesh = glCreateProgram();
	// 附加着色器对象
	glAttachShader(shader_mesh, shader_vertex);
	// 附加着色器对象
	glAttachShader(shader_mesh, shader_fragment_mesh);
	// 指明要被链接的程序对象句柄
	glLinkProgram(shader_mesh);

	// 着色器对象已附加到某个程序对象,则仅会将其标记为待删除，但不会立即删除。
	// 只有当该着色器对象从所有程序对象中分离，且在所有渲染上下文中均未被使用时，才会真正被删除。
	glDeleteShader(shader_vertex);
	glDeleteShader(shader_fragment_surface);
	glDeleteShader(shader_fragment_mesh);

	// ------------------------------------------------------------
	// create axis
	// ------------------------------------------------------------

	// glGenVertexArrays返回一个顶点数组对象的名称，并存在了VAO_AXIS中
	glGenVertexArrays(1, &VAO_AXIS);
	// glGenBuffers函数会生成1个缓冲区对象标识符，并将这些标识符存储在VBO_AXIS中。
	glGenBuffers(1, &VBO_AXIS);

	// glBindVertexArray函数用于绑定指定名称的顶点数组对象（VAO）
	glBindVertexArray(VAO_AXIS);
	// glBindBuffer用于绑定一个已经命名的缓存区对象。GL_ARRAY_BUFFER代表顶点属性
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
	
	// glBufferData是OpenGL中用于为缓冲区对象（Buffer Object）分配数据存储空间的函数
	// GL_ARRAY_BUFFER表示当前绑定的缓冲区对象所关联的目标类型为顶点属性（位置、颜色、纹理坐标）
	// 数组axis包含了用于指定要复制到缓冲区对象数据存储区的初始化数据
	/* GL_STATIC_DRAW要分开看: STATIC表示数据会被修改一次，利用许多次；
 		DRAW表明数据内容被应用程序所修改，并且数据内容作为OpenGL绘图和图像规范命令的来源
 	*/
	glBufferData(GL_ARRAY_BUFFER, sizeof(axis), axis, GL_STATIC_DRAW);

	// 定义一个通用顶点属性数据的数组
	/* 0表示需修改的通用顶点属性的索引
 	   3表示每个通用顶点属性的分量数目。
     	   GL_FLOAT表示数组中每个分量的数据类型为单精度浮点型
	   GL_FALSE表示当访问定点数据值时直接按原定点值转换
           指定连续通用顶点属性之间的字节偏移量（步幅）为6 * sizeof(float)
	*/
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// ------------------------------------------------------------
	// states
	// ------------------------------------------------------------

	glLineWidth(2);
	glPointSize(10);
	glEnable(GL_DEPTH_TEST);

	// ------------------------------------------------------------
	// ebo
	// ------------------------------------------------------------

	// GLuint e;
	glGenBuffers(1,&EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	ebo_update();

}

// ------------------------------------------------------------
// render event
// ------------------------------------------------------------


void CanvasGL::render(wxPaintEvent& event) {

  // ------------------------------------------------------------
  // check init
  // ------------------------------------------------------------

  // check if opengl (glad pointers) have been initialized. the paint
  // event may trigger this function without opengl being initialized?

  if (!gl_has_been_init) {
    init_gl();
    gl_has_been_init = true;
  }

  // ------------------------------------------------------------
  // set context and states
  // ------------------------------------------------------------

  SetCurrent(*m_context);
  wxPaintDC dc(this);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

  // ------------------------------------------------------------
  // transformations
  // ------------------------------------------------------------

  /* ----------- look at origin ----------- */

  glm::mat4 view = glm::mat4(1.0f);
  view = glm::lookAt(camera_pos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // dest, up

  /* --------- handle projection --------- */

  int width, height;
  GetClientSize(&width, &height);
  float aspectRatio = static_cast<float>(width) / static_cast<float>(height);

  glm::mat4 projection;
  if (props.perspective) {
    projection = glm::perspective(glm::radians(fov), aspectRatio, near_plane, far_plane);
  } else {
    float left = -ortho_size * aspectRatio;
    float right = ortho_size * aspectRatio;
    float bottom = -ortho_size;
    float top = ortho_size;
    projection = glm::ortho<float>(left, right, bottom, top, near_plane, far_plane);
  }

  /* ----- model (not needed for now) ----- */

  glm::mat4 model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));

  // ------------------------------------------------------------
  // draw surfaces
  // ------------------------------------------------------------

  glUseProgram(shader_surface);
  glEnable(GL_DEPTH_TEST);

  GLuint locView = glGetUniformLocation(shader_surface, "view");
  glUniformMatrix4fv(locView, 1, GL_FALSE, glm::value_ptr(view));
  GLuint locProjection = glGetUniformLocation(shader_surface, "projection");
  glUniformMatrix4fv(locProjection, 1, GL_FALSE, glm::value_ptr(projection));
  GLuint locModel = glGetUniformLocation(shader_surface, "model");
  glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));

  for (const auto& pair : surfaces_data) {
    if (!pair.second.show || pair.second.function.empty())
      continue;
    glBindVertexArray(pair.second.vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pair.second.ebo);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawElements(GL_TRIANGLES, pair.second.ind_size, GL_UNSIGNED_INT, 0);
  }

  // ------------------------------------------------------------
  // draw meshes
  // ------------------------------------------------------------

  if (props.show_mesh) {
    glUseProgram(shader_mesh);
    glEnable(GL_DEPTH_TEST);

    locView = glGetUniformLocation(shader_mesh, "view");
    glUniformMatrix4fv(locView, 1, GL_FALSE, glm::value_ptr(view));
    locProjection = glGetUniformLocation(shader_mesh, "projection");
    glUniformMatrix4fv(locProjection, 1, GL_FALSE, glm::value_ptr(projection));
    locModel = glGetUniformLocation(shader_mesh, "model");
    glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));

    glLineWidth(2);
    
    for (const auto& pair : surfaces_data) {
      if (!pair.second.show || pair.second.function.empty())
	continue;
      glBindVertexArray(pair.second.vao);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pair.second.ebo);
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      glDrawElements(GL_TRIANGLES, pair.second.ind_size, GL_UNSIGNED_INT, 0);
    }
  }

  // ------------------------------------------------------------
  // draw axes
  // ------------------------------------------------------------

  if (props.show_axes) {
    glUseProgram(shader_surface);
    glDisable(GL_DEPTH_TEST);
    glLineWidth(5);
    glBindVertexArray(VAO_AXIS);
    glDrawArrays(GL_LINES, 0, 6);
  }

  // ------------------------------------------------------------
  // display
  // ------------------------------------------------------------

  SwapBuffers();

}

// ------------------------------------------------------------
// size event
// ------------------------------------------------------------

void CanvasGL::on_size(wxSizeEvent &event) {
    int width, height;
    GetClientSize(&width, &height);
    // if glad has not been initialized, glViewport will try to
    // dereference a null func pointer.
    if (gl_has_been_init) {
      glViewport(0, 0, width, height);
    }
    Refresh();
}

// ------------------------------------------------------------
// mouse motion event
// ------------------------------------------------------------

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

// ------------------------------------------------------------
// left mouse button events
// ------------------------------------------------------------

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

// ------------------------------------------------------------
// right mouse button events
// ------------------------------------------------------------

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

// ------------------------------------------------------------
//
// ------------------------------------------------------------

void CanvasGL::ebo_update() {

  std::vector<unsigned int> ind;
  int num_vertices_per_axis = props.divisions + 1;
    
  // indices
  for (int i = 0; i < props.divisions; i++) {
    for (int j = 0; j < props.divisions; j++) {
      int row1 = i * num_vertices_per_axis;
      int row2 = (i + 1) * num_vertices_per_axis;
      // first quad
      ind.push_back(row1 + j);
      ind.push_back(row2 + j);
      ind.push_back(row1 + j + 1);
      // second quad
      ind.push_back(row1 + j + 1);
      ind.push_back(row2 + j);
      ind.push_back(row2 + j + 1);
    }
  }

  this->ind_size = ind.size();

  for (auto& pair : surfaces_data) {
    glBindVertexArray(pair.second.vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    // glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, ind.size() * sizeof(unsigned int), ind.data());
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, ind.size() * sizeof(unsigned int), ind.data(), GL_STATIC_DRAW);
    pair.second.ebo = EBO;
    pair.second.ind_size = ind.size();
  }
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
