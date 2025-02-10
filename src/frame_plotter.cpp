#include <wx/wx.h>
#include <wx/gbsizer.h>
#include <frame_plotter.hpp>
#include <window_surface_config.hpp>
#include <data_properties.hpp>
#include <data_surfaces.hpp>
#include <map>

// ------------------------------------------------------------
// frame plotter constructor
// ------------------------------------------------------------

FramePlotter::FramePlotter(wxFrame* parent)
  : wxFrame(parent, wxID_ANY, "Plotter3D") {

  this->SetMinClientSize(wxSize(1200, 600));

  // ------------------------------------------------------------
  // initialize structs
  // ------------------------------------------------------------

  /* ---- initialize properties struct ---- */

   props = {
     .grid_size = 20,
     .divisions = 20,
     .perspective = true,
     .show_axes = true,
     .show_mesh = true,
     // .lighting = true
  };

   /* ----------- initialize map ----------- */
   
   // surfaces map is empty.

  // ------------------------------------------------------------
  // main panel
  // ------------------------------------------------------------

  wxPanel* panel_main = new wxPanel(this);
  wxBoxSizer* sizer_main = new wxBoxSizer(wxHORIZONTAL);
  panel_main->SetSizer(sizer_main);
  
  // ------------------------------------------------------------
  // opengl area (left)
  // ------------------------------------------------------------

  int args[] = {WX_GL_CORE_PROFILE,
		WX_GL_MAJOR_VERSION, 3,
		WX_GL_MINOR_VERSION, 3,
		WX_GL_RGBA,
		WX_GL_DOUBLEBUFFER,
		WX_GL_DEPTH_SIZE, 16,
		0};
  canvas_gl = new CanvasGL(panel_main, args, props, surfaces_data);

  // ------------------------------------------------------------
  // configuration (right)
  // ------------------------------------------------------------

  wxPanel* panel_right = new wxPanel(panel_main);
  wxBoxSizer* sizer_right = new wxBoxSizer(wxVERTICAL);
  panel_right->SetSizer(sizer_right);

  /* ----------- scrolled area ----------- */

  panel_scrolled = new PanelScrolled(panel_right, props, surfaces_data);
  panel_scrolled->set_canvas_gl(canvas_gl);

  /* -------- staticbox properties -------- */
  
  wxStaticBox* staticbox_properties = new wxStaticBox(panel_right, wxID_ANY, "Properties");
  wxGridBagSizer* staticbox_sizer = new wxGridBagSizer();
  staticbox_properties->SetSizer(staticbox_sizer);

  wxString combobox_projection_choices[2] = {"Perspective", "Orthographic"};

  textctrl_gridsize   = new wxTextCtrl(staticbox_properties, wxID_ANY, "");
  textctrl_divisions  = new wxTextCtrl(staticbox_properties, wxID_ANY, "");
  checkbox_axes       = new wxCheckBox(staticbox_properties, wxID_ANY, "Show axes");
  checkbox_mesh       = new wxCheckBox(staticbox_properties, wxID_ANY, "Show mesh");
  // checkbox_lighting   = new wxCheckBox(staticbox_properties, wxID_ANY, "Lighting:");
  combobox_projection = new wxComboBox(staticbox_properties, wxID_ANY, "Perspective",
				       wxDefaultPosition, wxDefaultSize, 2,
				       combobox_projection_choices, wxCB_READONLY);

  /* --- set initial values to controls --- */

  textctrl_gridsize ->SetValue(wxString::Format(wxT("%d"), props.grid_size));
  textctrl_divisions->SetValue(wxString::Format(wxT("%.2f"), props.divisions));
  checkbox_axes     ->SetValue(props.show_axes);
  checkbox_mesh     ->SetValue(props.show_mesh);
  // checkbox_lighting ->SetValue(props.lighting);

  /* ------------ bind events ------------ */

  textctrl_gridsize  ->Bind(wxEVT_TEXT,     &FramePlotter::on_gridsize, this);
  textctrl_divisions ->Bind(wxEVT_TEXT,     &FramePlotter::on_divisions, this);
  checkbox_axes      ->Bind(wxEVT_CHECKBOX, &FramePlotter::on_axes, this);
  checkbox_mesh      ->Bind(wxEVT_CHECKBOX, &FramePlotter::on_mesh, this);
  // checkbox_lighting  ->Bind(wxEVT_CHECKBOX, &FramePlotter::on_lighting, this);
  combobox_projection->Bind(wxEVT_COMBOBOX, &FramePlotter::on_projection, this);

  /* ------------ add to sizer ------------ */
  
  staticbox_sizer->Add(new wxStaticText(staticbox_properties, wxID_ANY, "Grid Size:"),  wxGBPosition(0, 0), wxGBSpan(1, 1), wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL);
  staticbox_sizer->Add(new wxStaticText(staticbox_properties, wxID_ANY, "Divisions:"),  wxGBPosition(1, 0), wxGBSpan(1, 1), wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL);
  staticbox_sizer->Add(new wxStaticText(staticbox_properties, wxID_ANY, "Projection:"), wxGBPosition(2, 0), wxGBSpan(1, 1), wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL);
  staticbox_sizer->Add(textctrl_gridsize,   wxGBPosition(0, 1), wxGBSpan(1, 1), wxALL|wxALIGN_LEFT, 5);
  staticbox_sizer->Add(textctrl_divisions,  wxGBPosition(1, 1), wxGBSpan(1, 1), wxALL|wxALIGN_LEFT, 5);
  staticbox_sizer->Add(combobox_projection, wxGBPosition(2, 1), wxGBSpan(1, 1), wxALL|wxALIGN_LEFT, 5);
  staticbox_sizer->Add(checkbox_axes,       wxGBPosition(3, 1), wxGBSpan(1, 1), wxEXPAND);
  staticbox_sizer->Add(checkbox_mesh,       wxGBPosition(4, 1), wxGBSpan(1, 1), wxEXPAND);
  // staticbox_sizer->Add(checkbox_lighting,   wxGBPosition(5, 1), wxGBSpan(1, 1), wxEXPAND);

  staticbox_sizer->AddGrowableCol(0, 1);
  staticbox_sizer->AddGrowableCol(1, 1);

  staticbox_sizer->Layout();

  // add to sizer and layout
  
  sizer_right->Add(panel_scrolled, 6, wxEXPAND|wxTOP|wxLEFT, 10);
  sizer_right->Add(staticbox_properties, 4, wxEXPAND|wxALL, 10);
  sizer_right->Layout();

  /* -- add left and right to main panel -- */

  sizer_main->Add(canvas_gl, 10, wxEXPAND);
  sizer_main->Add(panel_right, 6, wxEXPAND);

  // ------------------------------------------------------------
  // menubar
  // ------------------------------------------------------------

  wxMenu *menu_file = new wxMenu;
  menu_file->Append(101, "&Exit\tCtrl-Q",
		   "Exit the program.");
  
  wxMenu *menu_surface = new wxMenu;
  menu_surface->Append(102, "&Add new surface...",
		       "");
 
  wxMenuBar *menuBar = new wxMenuBar;
  menuBar->Append(menu_file, "&File");
  menuBar->Append(menu_surface, "&Surfaces");
 
  SetMenuBar(menuBar);
 
  CreateStatusBar();
  SetStatusText("Welcome to Plotter3D");

  /* -------- bind menubar events -------- */

  Bind(wxEVT_MENU, &FramePlotter::on_menu_exit, this, 101);
  Bind(wxEVT_MENU, &FramePlotter::on_menu_surface, this, 102);
  
}

// ------------------------------------------------------------
// events
// ------------------------------------------------------------


void FramePlotter::on_gridsize(wxCommandEvent& event) {
  long value;
  textctrl_gridsize->GetValue().ToLong(&value);
  if (value <= 0) return;
  props.grid_size = (int)value;
  // update ebo
  canvas_gl->ebo_update();
  for (const auto& pair : surfaces_data) {
    // realloc buffer, and update all
    pair.second.window_surface_config->update_buffer_size();
    pair.second.window_surface_config->vector_update_coords();
    pair.second.window_surface_config->vector_update_colors();
    pair.second.window_surface_config->vector_send_to_buffer();
  }
  canvas_gl->Refresh();
}

void FramePlotter::on_divisions(wxCommandEvent& event) {
  double value;
  textctrl_divisions->GetValue().ToDouble(&value);
  if (value <= 1) return;
  props.divisions = (float)value;
  // update ebo
  canvas_gl->ebo_update();
  for (const auto& pair : surfaces_data) {
    // realloc buffer, and update all
    pair.second.window_surface_config->update_buffer_size();
    pair.second.window_surface_config->vector_update_coords();
    pair.second.window_surface_config->vector_update_colors();
    pair.second.window_surface_config->vector_send_to_buffer();
  }
  canvas_gl->Refresh();
}

void FramePlotter::on_projection(wxCommandEvent& event) {
  if (combobox_projection->GetValue() == wxString("Perspective")) {
    props.perspective = true;
  } else {
    props.perspective = false;
  }
  canvas_gl->Refresh();
}

void FramePlotter::on_axes(wxCommandEvent& event) {
  props.show_axes = checkbox_axes->GetValue();
  canvas_gl->Refresh();
}

void FramePlotter::on_mesh(wxCommandEvent& event) {
  props.show_mesh = checkbox_mesh->GetValue();
  canvas_gl->Refresh();
}

// void FramePlotter::on_lighting(wxCommandEvent& event) {
//   props.lighting = checkbox_lighting->GetValue();
//   canvas_gl->Refresh();
// }

void FramePlotter::on_menu_exit(wxCommandEvent &event) {
  Close(true);
}

void FramePlotter::on_menu_surface(wxCommandEvent& event) {
  WindowSurfaceConfig* window_surface_config = this->panel_scrolled->create_surface_config_window();
  window_surface_config->set_canvas_gl(canvas_gl);
}
