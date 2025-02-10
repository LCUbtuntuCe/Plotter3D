#pragma once

#include <wx/wx.h>
#include <renderer.hpp>
#include <data_properties.hpp>
#include <data_surfaces.hpp>
#include <window_surface_config.hpp>
#include <string>
#include <map>

// ------------------------------------------------------------
// scrollable panel for WindowSurfaceConfig
// ------------------------------------------------------------

class PanelScrolled : public wxScrolled<wxPanel> {
  unsigned int id_count = 0;
  Properties& props;
  std::map<unsigned int, SurfaceData>& surfaces_data;
  wxBoxSizer* sizer;
public:

  // ------------------------------------------------------------
  // constructor
  // ------------------------------------------------------------
  
  PanelScrolled(wxWindow* parent, Properties& props, std::map<unsigned int, SurfaceData>& surfaces_data)
    : wxScrolled(parent, wxID_ANY),
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
      .ind_size = 0
    };

    surfaces_data[id_new] = surface_new;

    glGenVertexArrays(1, &surfaces_data[id_new].vao);
    glGenBuffers(1, &surfaces_data[id_new].vbo);

    glBindVertexArray(surfaces_data[id_new].vao);
    glBindBuffer(GL_ARRAY_BUFFER, surfaces_data[id_new].vbo);
    // set location and data format
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, surface_new.ebo);

    window_surface_config->update_buffer_size();
    window_surface_config->vector_update_colors();
    
    /* ---------- add to scrolled ---------- */
    
    sizer->Add(window_surface_config, 0, wxALL|wxEXPAND, 5);
    this->Layout();
    this->FitInside();

    return window_surface_config;
  }
  
};

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
