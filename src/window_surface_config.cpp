#include <wx/wx.h>
#include <window_surface_config.hpp>
#include <wx/clrpicker.h>

WindowSurfaceConfig::WindowSurfaceConfig(wxPanel* parent)
  : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize) {

  wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
  this->SetSizer(sizer);

  wxCheckBox* checkbox_show = new wxCheckBox(this, wxID_ANY, "Show");
  wxTextCtrl* textctrl_function = new wxTextCtrl(this, wxID_ANY, "");
  wxButton* button_remove = new wxButton(this, wxID_ANY, "Remove");
  wxColourPickerCtrl* colour_picker = new wxColourPickerCtrl(this, wxID_ANY, wxColour(255, 0, 0));

  sizer->Add(checkbox_show, 0, wxALL|wxEXPAND, 5);
  sizer->Add(new wxStaticText(this, wxID_ANY, "f(x,y) = "), 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
  sizer->Add(textctrl_function, 1, wxALL|wxEXPAND, 5);
  sizer->Add(colour_picker, 0, wxALL|wxEXPAND, 5);
  sizer->Add(button_remove, 0, wxALL|wxEXPAND, 5);

  sizer->Layout();
}
