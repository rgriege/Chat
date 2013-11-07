#ifndef __GUI_H__
#define __GUI_H__

#include "Interface.h"

#ifndef __Win32
#include "CEGUI\CEGUI.h"
#else
#include "CEGUI.h"
#endif

class Gui : public Interface
{
public:
  Gui();
  void Run();
  void ShowMessage(std::string msg);

private:
  void SetupLogin();
  void Display();
  void CheckInput();
  void CheckServer();

  void handle_mouse_down(char button);
  void handle_mouse_up(char button);
  void subscribeEvent(const CEGUI::String& widget, const CEGUI::String& event, const CEGUI::Event::Subscriber& method);

  bool SetupChat(const CEGUI::EventArgs &args);
  bool ClearEditbox(const CEGUI::EventArgs &args);
  bool Quit(const CEGUI::EventArgs &args);

  CEGUI::Listbox* messageBox;
  CEGUI::MultiLineEditbox* inputBox;
  bool loggedIn;
};

#endif // __GUI_H__
