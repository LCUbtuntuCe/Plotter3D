#pragma once

#include <wx/wx.h>
#include <renderer.hpp>
#include <data_properties.hpp>
#include <data_surfaces.hpp>
#include <window_surface_config.hpp>
#include <string>
#include <map>

// ------------------------------------------------------------
// 用于 `WindowSurfaceConfig` 的可滚动面板
// ------------------------------------------------------------

// 可滚动窗口类wxScrolledWindow继承自wxPanel
class PanelScrolled : public wxScrolledWindow {
  unsigned int id_count = 0;
  Properties& props;
  std::map<unsigned int, SurfaceData>& surfaces_data;
  wxBoxSizer* sizer;
  CanvasGL* canvas_gl = nullptr;
public:

  // ------------------------------------------------------------
  // constructor
  // ------------------------------------------------------------
  
  PanelScrolled(wxWindow* parent, Properties& props, std::map<unsigned int, SurfaceData>& surfaces_data)
    : wxScrolledWindow(parent, wxID_ANY),
      props(props),
      surfaces_data(surfaces_data) {
    
    sizer = new wxBoxSizer(wxVERTICAL);

    this->SetSizer(sizer);
    this->FitInside();
    this->SetScrollRate(5, 5);
  }

  // ------------------------------------------------------------
  // create new config window
  // ------------------------------------------------------------

  WindowSurfaceConfig* create_surface_config_window() {

    unsigned int id_new = this->id_count++;

    /* ----------- create window ----------- */
    
    WindowSurfaceConfig* window_surface_config = new WindowSurfaceConfig(this, id_new, props, surfaces_data);

    /* ----------- init map entry ----------- */

    SurfaceData surface_new = {
      .function = "",
      .show = true,
      .rgb = {1.0f, 0.0f, 0.0f},
      .ind_size = 0,
      .window_surface_config = window_surface_config
    };

    surfaces_data[id_new] = surface_new;

    glGenVertexArrays(1, &surfaces_data[id_new].vao);
    glGenBuffers(1, &surfaces_data[id_new].vbo);

    glBindVertexArray(surfaces_data[id_new].vao);
    glBindBuffer(GL_ARRAY_BUFFER, surfaces_data[id_new].vbo);
    
    if (canvas_gl) {
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, canvas_gl->EBO);
      surfaces_data[id_new].ind_size = canvas_gl->ind_size;
    }
    
    // set location and data format
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    window_surface_config->update_buffer_size();
    window_surface_config->vector_update_colors();
    
    /* ---------- add to scrolled ---------- */
    
    sizer->Add(window_surface_config, 0, wxALL|wxEXPAND, 5);
    this->Layout();
    this->FitInside();

    return window_surface_config;
  }

  // ------------------------------------------------------------
  // assign canvas gl
  // ------------------------------------------------------------

  void set_canvas_gl(CanvasGL* canvas_gl) {
    this->canvas_gl = canvas_gl;
  }

};

// wxFrame是一个框架类，表示一个窗口其大小和位置可以由用户更改
// 它通常具有粗边框和标题栏，并且可以选择包含菜单栏、工具栏和状态栏。一个框架可以容纳任何不是框架或对话框的窗口。
class FramePlotter : public wxFrame {
  Properties props;
  std::map<unsigned int, SurfaceData> surfaces_data;
  CanvasGL* canvas_gl;
  PanelScrolled* panel_scrolled;
  wxTextCtrl* textctrl_gridsize;
  wxTextCtrl* textctrl_divisions;
  wxCheckBox* checkbox_axes;
  wxCheckBox* checkbox_mesh;
  // wxCheckBox* checkbox_lighting;
  wxComboBox* combobox_projection;
public:
  FramePlotter(wxFrame* parent);
  void on_gridsize(wxCommandEvent& event);
  void on_divisions(wxCommandEvent& event);
  void on_projection(wxCommandEvent& event);
  void on_axes(wxCommandEvent& event);
  void on_mesh(wxCommandEvent& event);
  // void on_lighting(wxCommandEvent& event);
  void on_menu_exit(wxCommandEvent& event);
  void on_menu_surface(wxCommandEvent& event);
  void help(WindowSurfaceConfig& window);

  friend class WindowSurfaceConfig;
};
