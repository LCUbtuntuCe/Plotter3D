#include <wx/wx.h>
#include <frame_plotter.hpp>

// wxApp派生自wxAppConsole

// 当wxUSE_GUI=1时，wxApp类表示应用程序本身
// 除了由wxAppConsole提供的功能之外，它还会跟踪顶级窗口(参见SetTopWindow())并增加对视频模式的支持(参见 SetDisplayMode())。
// 通常，仅限 GUI 的应用程序的全局设置可以通过 wxApp（或 wxSystemSettings 或 wxSystemOptions 类）访问。
class AppPlotter: public wxApp {
public:
  // 重写基类虚函数
  bool OnInit() override {
    // wxFrame是一个框架类，表示一个窗口其大小和位置可以由用户更改
    // 它通常具有粗边框和标题栏，并且可以选择包含菜单栏、工具栏和状态栏。一个框架可以容纳任何不是框架或对话框的窗口。
    wxFrame* frame_plotter = new FramePlotter(nullptr);
    // 显示主窗口
    frame_plotter->Show(true);
    return true;
  }
};

// 这个宏定义了应用程序的入口点，并告诉wxWidgets应该使用哪个应用程序类。
wxIMPLEMENT_APP(AppPlotter);
