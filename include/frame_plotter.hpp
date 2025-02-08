#pragma once

#include <wx/wx.h>
#include <renderer.hpp>
#include <data_properties.hpp>

class FramePlotter : public wxFrame {
  Properties props;
  CanvasGL* canvas_gl;
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
};
