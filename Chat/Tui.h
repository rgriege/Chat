#ifndef __TUI_H__
#define __TUI_H__

#include <SDL_mutex.h>

#include "Interface.h"

class Tui : public Interface
{
public:
  Tui(std::string host, std::string name);
  void Run();
  void CheckServer();
  void ShowMessage(std::string msg);

private:
	std::string buffer;
	bool buffering;
	SDL_mutex* bufferLock;
};

#endif // __TUI_H__
