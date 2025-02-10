#pragma once

#include <wx/event.h>
#include <wx/wx.h>
#include <wx/clrpicker.h>
#include <data_surfaces.hpp>
#include <data_properties.hpp>
#include <map>
class CanvasGL;

class WindowSurfaceConfig : public wxPanel {
  unsigned int id;
  Properties& props;
  std::map<unsigned int, SurfaceData>& surfaces_data;
  wxTextCtrl* textctrl_function;
  wxCheckBox* checkbox_show;
  wxColourPickerCtrl* colour_picker;
  wxButton* button_remove;
  CanvasGL* canvas_gl = nullptr;
public:
  WindowSurfaceConfig(wxPanel* parent, unsigned int id, Properties& props, std::map<unsigned int, SurfaceData>& surfaces_data);
  void on_checkbox(wxCommandEvent& event);
  void on_textctrl(wxCommandEvent& event);
  void on_color(wxColourPickerEvent& event);
  void on_remove(wxCommandEvent& event);
  void update_buffer_size();
  void vector_update_colors();
  void vector_update_coords();
  void vector_send_to_buffer();
  void set_canvas_gl(CanvasGL* canvas_gl);
};
