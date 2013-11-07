#ifndef __INTERFACE_H__
#define __INTERFACE_H__

#include <string>

class Interface
{
public:
  Interface() {}
  virtual void Run() = 0;
  virtual void ShowMessage(std::string msg) = 0;

protected:
  bool quit;
};

#endif // __INTERFACE_H__
