#include "Tui.h"

#include <iostream>
#include <conio.h>

#include "NetworkManager.h"

using namespace std;

Tui::Tui(std::string host, std::string name) : buffering(false)
{
	/* If no host is specified, start the server */
	if (host.length() == 0)
		Server::GetInstance();

	NetworkManager::GetInstance()->ConnectToServer(host);

	if (name.length() > 0) {
		const char* cName = name.c_str();
		char *msg = new char[name.length()+2];
		msg[0] = NAME;
		strcpy_s(&msg[1], strlen(cName)+1, cName);
		NetworkManager::GetInstance()->SendMessage(msg);
		delete[] msg;
	}
}

void Tui::Run()
{
	char msg[512];
	quit = false;
	while (!quit) {
		/* Spin until keyboard input */
		/* All of the following block for return and thus can't be used:
		 * getchar(), getc(stdin), cin.get(), cin.ignore() */
		//cout << (msg[1] = _getch());
		while (!_kbhit())
			SDL_Delay(100);

		/* Send incoming messages to the buffer while typing */
		SDL_mutexP(bufferLock);
		buffering = true;
		SDL_mutexV(bufferLock);

		/* Block until the message is complete */
		msg[0] = MESSAGE;
		cin.getline (&msg[1], 100);

		/* Dump the buffer */
		SDL_mutexP(bufferLock);
		buffering = false;
		cout << buffer;
		buffer.clear();
		SDL_mutexV(bufferLock);

		if(strcmp(&msg[1], "exit") == 0 || strcmp(&msg[1], "quit") == 0) {
			quit = true;
			msg[0] = QUIT;
			msg[1] = 0;
			NetworkManager::GetInstance()->SendMessage(msg);
		} else {
			NetworkManager::GetInstance()->SendMessage(std::string(msg));
		}
	}
}

void Tui::ShowMessage(std::string msg)
{
	SDL_mutexP(bufferLock);
	if (buffering) {
		buffer += msg + "\n";
		SDL_mutexV(bufferLock);
	} else {
		SDL_mutexV(bufferLock);
		cout << msg << endl;
	}
}