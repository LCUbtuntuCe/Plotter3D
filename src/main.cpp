#include "../include/renderer.hpp"
#include <wx/wx.h>
#include <wx/gdicmn.h>
#include <wx/colour.h>
#include <wx/scrolwin.h>
#include <wx/statbox.h>
#include <wx/gbsizer.h>
#include <wx/event.h>
#include <wx/glcanvas.h>
#include <wx/statbox.h>
#include <wx/textctrl.h>
#include <wx/combobox.h>
#include <wx/checkbox.h>

/* -------------------- scrolled panel -------------------- */

class PanelScrolled : public wxScrolled<wxPanel> {
public:
  PanelScrolled(wxWindow* parent)
    : wxScrolled(parent, wxID_ANY) {
    
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    for (int i=0; i<50; i++) {
      sizer->Add(new wxButton(this, wxID_ANY, wxString::Format("Button %d", i)), 0, wxALL, 5);
    }

    this->SetSizer(sizer);
    this->FitInside();
    this->SetScrollRate(5, 5);
  }
};

/* ---------------------- main frame ---------------------- */

class FramePlotter : public wxFrame {
public:
  FramePlotter(wxFrame* parent);
};

FramePlotter::FramePlotter(wxFrame *parent)
  : wxFrame(parent, wxID_ANY, "Plotter3D") {
  
  this->SetMinClientSize(wxSize(800, 600));

  /* ---------------------- main panel ---------------------- */

  wxPanel* panel_main = new wxPanel(this);
  wxBoxSizer* sizer_main = new wxBoxSizer(wxHORIZONTAL);
  panel_main->SetSizer(sizer_main);

  /* -------------- left side (render canvas) -------------- */

  int args[] = {WX_GL_CORE_PROFILE,
		WX_GL_MAJOR_VERSION, 3,
		WX_GL_MINOR_VERSION, 3,
		WX_GL_RGBA,
		WX_GL_DOUBLEBUFFER,
		WX_GL_DEPTH_SIZE, 16,
		0};
  CanvasGL* canvas_gl = new CanvasGL(panel_main, args);

  /* ------------- right panel (configuration) ------------- */

  wxPanel* panel_right = new wxPanel(panel_main);
  panel_right->SetBackgroundColour(wxColour(255, 0, 0));
  wxBoxSizer* sizer_right = new wxBoxSizer(wxVERTICAL);
  panel_right->SetSizer(sizer_right);

  // scrolled

  PanelScrolled* panel_scrolled = new PanelScrolled(panel_right);

  // staticbox
  
  wxStaticBox* staticbox_properties = new wxStaticBox(panel_right, wxID_ANY, "Properties");
  wxGridBagSizer* staticbox_sizer = new wxGridBagSizer();
  staticbox_properties->SetSizer(staticbox_sizer);

  wxString combobox_projection_choices[2] = {"Perspective", "Orthographic"};

  wxTextCtrl* textctrl_gridsize   = new wxTextCtrl(staticbox_properties, wxID_ANY, "50");
  wxTextCtrl* textctrl_resolution = new wxTextCtrl(staticbox_properties, wxID_ANY, "0.1");
  wxCheckBox* checkbox_axes       = new wxCheckBox(staticbox_properties, wxID_ANY, "Show axes:");
  wxCheckBox* checkbox_mesh       = new wxCheckBox(staticbox_properties, wxID_ANY, "Show mesh:");
  wxCheckBox* checkbox_lighting   = new wxCheckBox(staticbox_properties, wxID_ANY, "Lighting:");
  wxComboBox* combobox_projection = new wxComboBox(staticbox_properties, wxID_ANY, "Perspective",
						   wxDefaultPosition, wxDefaultSize, 2,
						   combobox_projection_choices, wxCB_READONLY);

  checkbox_axes->SetValue(true);
  checkbox_mesh->SetValue(true);
  checkbox_lighting->SetValue(true);
  
  staticbox_sizer->Add(new wxStaticText(staticbox_properties, wxID_ANY, "Grid Size:"),  wxGBPosition(0, 0), wxGBSpan(1, 1), wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL);
  staticbox_sizer->Add(new wxStaticText(staticbox_properties, wxID_ANY, "Resolution:"), wxGBPosition(1, 0), wxGBSpan(1, 1), wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL);
  staticbox_sizer->Add(new wxStaticText(staticbox_properties, wxID_ANY, "Projection:"), wxGBPosition(2, 0), wxGBSpan(1, 1), wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL);
  staticbox_sizer->Add(textctrl_gridsize,   wxGBPosition(0, 1), wxGBSpan(1, 1), wxALIGN_CENTER|wxALIGN_LEFT);
  staticbox_sizer->Add(textctrl_resolution, wxGBPosition(1, 1), wxGBSpan(1, 1), wxALIGN_CENTER|wxALIGN_LEFT);
  staticbox_sizer->Add(combobox_projection, wxGBPosition(2, 1), wxGBSpan(1, 1), wxALIGN_CENTER|wxALIGN_LEFT);
  staticbox_sizer->Add(checkbox_axes,       wxGBPosition(3, 1), wxGBSpan(1, 1), wxEXPAND);
  staticbox_sizer->Add(checkbox_mesh,       wxGBPosition(4, 1), wxGBSpan(1, 1), wxEXPAND);
  staticbox_sizer->Add(checkbox_lighting,   wxGBPosition(5, 1), wxGBSpan(1, 1), wxEXPAND);

  staticbox_sizer->AddGrowableCol(0, 1);
  staticbox_sizer->AddGrowableCol(1, 1);

  staticbox_sizer->Layout();

  // add to sizer and layout
  
  sizer_right->Add(panel_scrolled, 1, wxEXPAND|wxTOP|wxLEFT, 10);
  sizer_right->Add(staticbox_properties, 1, wxEXPAND|wxALL, 10);
  sizer_right->Layout();

  /* -------------- add left and right to main -------------- */

  sizer_main->Add(canvas_gl, 3, wxEXPAND);
  sizer_main->Add(panel_right, 1, wxEXPAND);
}

/* ------------------------ wx app ------------------------ */

class AppPlotter: public wxApp {
public:
  bool OnInit() override {
    wxFrame* frame_plotter = new FramePlotter(nullptr);
    frame_plotter->Show(true);
    return true;
  }
};

wxIMPLEMENT_APP(AppPlotter);
