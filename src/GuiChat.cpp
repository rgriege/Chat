#include <iostream>

#include "NetworkManager.h"
#include "Interface.h"
#include "Gui.h"
#include "Tui.h"
#include "Server.h"
#include "constants.h"

using namespace std;

Interface *interface;

void *showMessagesLoop(void *ptr);

int main(int argc, char **argv)
{
  /* Parameter checking */
  if (argc > 6) {
    fprintf(stderr, "Usage: %s [-h host][-n name][-t]\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  bool useGui = true;
  string host;
  string name = "Stranger";
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-h") == 0) {
      if (i + 1 < argc)
        host = argv[++i];
      else
        cerr << "-h requires an argument" << endl;
    } else if (strcmp(argv[i], "-n") == 0) {
      if (i + 1 < argc)
        name = argv[++i];
      else
        cerr << "-n requires an argument" << endl;
    } else if (strcmp(argv[i], "-t") == 0) {
      useGui = false;
    }
  }

  /* Start the appropriate interface */
  if (useGui) { 
    interface = new Gui();
  } else {
    interface = new Tui(host, name);
  }

  interface->Run();

  Server::GetInstance()->Stop();

  NetworkManager::GetInstance()->Close();

  return EXIT_SUCCESS;
}

void *showMessagesLoop(void *ptr)
{
  int quit = 0;
  while (!quit) {
    string msg = NetworkManager::GetInstance()->ReceiveMessage();
    interface->ShowMessage(msg);
  }
  return NULL;
}

