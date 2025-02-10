#include <wx/clrpicker.h>
#include <wx/event.h>
#include <wx/wx.h>
#include <window_surface_config.hpp>
#include <renderer.hpp>
#include <parser.hpp>

WindowSurfaceConfig::WindowSurfaceConfig(wxPanel* parent, unsigned int id, Properties& props, std::map<unsigned int, SurfaceData>& surfaces_data)
  : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize),
    id(id),
    props(props),
    surfaces_data(surfaces_data) {

  wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
  this->SetSizer(sizer);

  checkbox_show = new wxCheckBox(this, wxID_ANY, "Show");
  textctrl_function = new wxTextCtrl(this, wxID_ANY, "");
  colour_picker = new wxColourPickerCtrl(this, wxID_ANY, wxColour(255, 0, 0));
  button_remove = new wxButton(this, wxID_ANY, "Remove");

  checkbox_show->SetValue(true);
  
  sizer->Add(checkbox_show, 0, wxALL|wxEXPAND, 5);
  sizer->Add(new wxStaticText(this, wxID_ANY, "f(x,y) = "), 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
  sizer->Add(textctrl_function, 1, wxALL|wxEXPAND, 5);
  sizer->Add(colour_picker, 0, wxALL|wxEXPAND, 5);
  sizer->Add(button_remove, 0, wxALL|wxEXPAND, 5);

  sizer->Layout();

  // ------------------------------------------------------------
  // events
  // ------------------------------------------------------------

  textctrl_function->Bind(wxEVT_TEXT, &WindowSurfaceConfig::on_textctrl, this);
  checkbox_show->Bind(wxEVT_CHECKBOX, &WindowSurfaceConfig::on_checkbox, this);
  colour_picker->Bind(wxEVT_COLOURPICKER_CHANGED, &WindowSurfaceConfig::on_color, this);
  button_remove->Bind(wxEVT_BUTTON, &WindowSurfaceConfig::on_remove, this);
}

void WindowSurfaceConfig::on_checkbox(wxCommandEvent& event) {
  surfaces_data[id].show = checkbox_show->GetValue();
  // refresh context
  if (canvas_gl) canvas_gl->Refresh();
}

void WindowSurfaceConfig::on_textctrl(wxCommandEvent& event) {
  // get function string
  surfaces_data[id].function = std::string(textctrl_function->GetValue().mb_str());
  // update ebo?
  canvas_gl->ebo_update();
  // update only xyz values
  this->vector_update_coords();
  // update buffer
  this->vector_send_to_buffer();
  // refresh context
  if (canvas_gl) canvas_gl->Refresh();
}

void WindowSurfaceConfig::on_color(wxColourPickerEvent& event) {
  // get color value
  wxColor color = colour_picker->GetColour();
  // normalize colors for opengl
  surfaces_data[id].rgb[0] = color.GetRed() / 255.0f;
  surfaces_data[id].rgb[1] = color.GetGreen() / 255.0f;
  surfaces_data[id].rgb[2] = color.GetBlue() / 255.0f;
  // update only rgb values
  this->vector_update_colors();
  // update buffer
  this->vector_send_to_buffer();
  // refresh context
  if (canvas_gl) canvas_gl->Refresh();
}

void WindowSurfaceConfig::on_remove(wxCommandEvent& event) {
  // delete vao and vbo
  glDeleteVertexArrays(1, &surfaces_data[id].vao);
  glDeleteBuffers(1, &surfaces_data[id].vbo);
  // remove from map
  surfaces_data.erase(id);
  // remove window
  this->GetParent()->GetSizer()->Detach(this);
  this->Destroy();
}

void WindowSurfaceConfig::update_buffer_size() {

  // resizes the array buffer

  // the vertices per axis (horizontal plane) are props.divisions +
  // 1. therefore, the total amount of vertices in the surface will be
  // this value squared. except that we also save the color data in
  // the array buffer, so we should multiply this by 6 (xyz + rgb per
  // vertex)
  unsigned int vertices_count = (props.divisions + 1) * (props.divisions + 1) * 6;

  surfaces_data[id].vertices.resize(vertices_count);
  // surfaces_data[id].ind_size = vertices_count;

  glBindVertexArray(surfaces_data[id].vao);
  glBindBuffer(GL_ARRAY_BUFFER, surfaces_data[id].vbo);
  glBufferData(GL_ARRAY_BUFFER, vertices_count * sizeof(float), NULL, GL_STATIC_DRAW);

  // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void WindowSurfaceConfig::vector_update_coords() {

  // recalculates xyz in vector

  parser p;
  char* expression = new char[surfaces_data[id].function.size() + 1];
  std::strcpy(expression, surfaces_data[id].function.c_str());

  float start = -props.grid_size / 2.0f;
  int num_vertices_per_axis = props.divisions + 1;
  double step = props.grid_size / props.divisions;

  for (int i = 0; i < num_vertices_per_axis; ++i) {
    for (int j = 0; j < num_vertices_per_axis; ++j) {
      float x = start + i * step;
      float y = start + j * step;

      p.set_xy(static_cast<double>(x), static_cast<double>(y));
      double z = p.eval_expr(expression);

      int index = (i * num_vertices_per_axis + j) * 6;      
      surfaces_data[id].vertices[index] = x;
      surfaces_data[id].vertices[index + 1] = static_cast<float>(z);
      surfaces_data[id].vertices[index + 2] = y;
    }
  }  
}

void WindowSurfaceConfig::vector_update_colors() {

  // recalculates rgb in vector

  // only update color values in the vertices data vector. since the
  // current format is xyzrgb, we need to skip every six indices.

  for (int i=3; i<surfaces_data[id].vertices.size(); i=i+6) {
    surfaces_data[id].vertices[i] = surfaces_data[id].rgb[0];
    surfaces_data[id].vertices[i+1] = surfaces_data[id].rgb[1];
    surfaces_data[id].vertices[i+2] = surfaces_data[id].rgb[2];
  }
}

void WindowSurfaceConfig::vector_send_to_buffer() {
  
  glBindVertexArray(surfaces_data[id].vao);
  glBindBuffer(GL_ARRAY_BUFFER, surfaces_data[id].vbo);
  // replace old data
  glBufferSubData(GL_ARRAY_BUFFER, 0, surfaces_data[id].vertices.size() * sizeof(float), surfaces_data[id].vertices.data());
  // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);  
}

void WindowSurfaceConfig::set_canvas_gl(CanvasGL* canvas_gl) {
  this->canvas_gl = canvas_gl;
}
