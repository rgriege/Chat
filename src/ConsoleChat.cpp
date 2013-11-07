#include <iostream>

#include "NetworkManager.h"
#include "Interface.h"
#include "Tui.h"
#include "Server.h"
#include "constants.h"

using namespace std;

Interface *interface;

bool quitListening = false;
int showMessagesLoop(void *ptr);

#undef main
int main(int argc, char **argv)
{
	/* Parameter checking */
	if (argc > 5) {
		fprintf(stderr, "Usage: %s [-h host][-n name]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

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
		}
	}

	/* Optionally start the server */
	bool isHost = host.length() == 0;
	if (isHost)
		Server::GetInstance();

	/* Start the appropriate interface */
	interface = new Tui(host, name);

	/* Start the listening thread */
	SDL_Thread* listenThread = SDL_CreateThread(showMessagesLoop, NULL);

	/* Run the main interface input loop */
	interface->Run();
	
	/* Optionally close the server */
	if (isHost)
		Server::GetInstance()->Stop();

	/* Close the listening thread */
	//quitListening = true;
	SDL_WaitThread(listenThread, NULL);

	/* Close the networking library */
	NetworkManager::GetInstance()->Close();

	return EXIT_SUCCESS;
}

int showMessagesLoop(void* ptr)
{
	while (!quitListening) {
		if (NetworkManager::GetInstance()->CheckSocket()) {
			string msg = NetworkManager::GetInstance()->ReceiveMessage();
			interface->ShowMessage(msg);
			if (msg == Server::leaveMessage || msg == Server::quitMessage)
				quitListening = true;
		}
	}
	return 0;
}