#include <wx/wx.h>
#include <frame_plotter.hpp>

class AppPlotter: public wxApp {
public:
  bool OnInit() override {
    wxFrame* frame_plotter = new FramePlotter(nullptr);
    frame_plotter->Show(true);
    return true;
  }
};

wxIMPLEMENT_APP(AppPlotter);
