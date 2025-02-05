#include <wx/wx.h>

class FramePlotter : public wxFrame {
public:
  FramePlotter(wxFrame* parent)
    : wxFrame(parent, wxID_ANY, "3DPlotter") {
    this->SetMinClientSize(wxSize(500, 500));
  }
};

class AppPlotter: public wxApp {
public:
  bool OnInit() override {
    wxFrame* frame_plotter = new FramePlotter(nullptr);
    frame_plotter->Show(true);
    return true;
  }
};

wxIMPLEMENT_APP(AppPlotter);
