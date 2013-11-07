#ifndef __SERVER_H__
#define __SERVER_H__

#include "SDL_net.h"
#include "constants.h"
#include <string>
#include "SDL_thread.h"

#define MAX_HOSTS 4

struct host {
  TCPsocket sock;
  IPaddress addr;
  std::string name;
};

class Server {
private:
  Server();
  Server(Server const&);
  Server& operator=(Server const&);

public:
  static Server* GetInstance();
  std::string getHostname();
  void Stop();

  static std::string quitMessage;
  static std::string leaveMessage;

private:
  void Start();
  void AddClient();
  void HandleClient(int idx);
  void RemoveClient(int idx);

  friend int createServer(void *prt);

  static Server* instance;
  static SDL_Thread* thread;
  static TCPsocket serversocket;
  static host hosts[MAX_HOSTS];
  static SDLNet_SocketSet socketset;
  static std::string hostname;
  static bool running;
};

#endif // __SERVER_H__
