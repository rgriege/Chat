#include "Server.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include <time.h>

#include "NetworkManager.h"

using namespace std;

Server* Server::instance = NULL;
SDL_Thread* Server::thread;
TCPsocket Server::serversocket;
host Server::hosts[MAX_HOSTS];
SDLNet_SocketSet Server::socketset;
string Server::hostname;
bool Server::running;
string Server::quitMessage = "The chatroom has been closed by the host.";
string Server::leaveMessage = "You have left the chatroom.";

int createServer(void *ptr)
{
	Server::GetInstance()->Start();
	return 0;
}

Server::Server()
{
	/* Make sure SDLNet is initialized */
	NetworkManager::GetInstance();

	IPaddress ip;

	/* Initialize the channels */
	for (int i = 0; i < MAX_HOSTS; i++) {
		hosts[i].sock = NULL;
	}

	/* Allocate the socket set */
	socketset = SDLNet_AllocSocketSet(MAX_HOSTS+1);
	if (socketset == NULL) {
		fprintf(stderr, "Server: Couldn't create socket set %s\n", SDLNet_GetError());
		exit(EXIT_FAILURE);
	}

	/* Resolving the host using NULL make network interface to listen */
	if (SDLNet_ResolveHost(&ip, NULL, GAME_PORT) < 0 || ip.host == INADDR_NONE) {
		fprintf(stderr, "Server: SDLNet_ResolveHost: %s\n", SDLNet_GetError());
		exit(EXIT_FAILURE);
	}

	/* Open a connection with the IP provided (listen on the host's port) */
	if (!(serversocket = SDLNet_TCP_Open(&ip))) {
		fprintf(stderr, "Server: SDLNet_TCP_Open: %s\n", SDLNet_GetError());
		exit(EXIT_FAILURE);
	}
	SDLNet_TCP_AddSocket(socketset, serversocket);

	//const char* compname = NetworkManager::GetHostname().c_str();
	//cout << "You are hosting a chat on '" << compname << "'" << endl << flush;
	hostname = SDLNet_ResolveIP(&ip);
	cout << "hosting on " << hostname << " from " << printIP(ip) << endl;

	srand(time(NULL));
}

Server* Server::GetInstance()
{
	if (!instance) {
		instance = new Server();
		SDL_CreateThread(createServer, NULL);
	}
	return instance;
}

void Server::Start()
{
	/* Wait for a connection, send data and term */
	running = true;
	while (running) {
		/* Wait for events */
		SDLNet_CheckSockets(socketset, TIMEOUT);
		/* Check for new connections */
		if (SDLNet_SocketReady(serversocket)) {
			AddClient();
		}
		/* Check for events on existing clients */
		for (int i = 0; i < MAX_HOSTS; i++) {
			if (SDLNet_SocketReady(hosts[i].sock)) {
				HandleClient(i);
			}
		}
	}

	for (int i = 0; i < MAX_HOSTS; i++)
		if (hosts[i].sock)
			NetworkManager::GetInstance()->SendMessage(quitMessage, hosts[i].sock);

	return;
}

void Server::HandleClient(int idx)
{
	char data[512];
	/* Has the connection been closed? */
	if (SDLNet_TCP_Recv(hosts[idx].sock, data, 512) <= 0) {
		RemoveClient(idx);
		/*unsigned count = 0;
		for (int i = 0; i < MAX_HOSTS; i++)
		if (hosts[i].sock) count++;
		if (count == 0)
		quit = 1;*/
	} else {
		if (data[TYPE] == NAME) {
			hosts[idx].name = string(&data[1]);
			if (hosts[idx].name == "Stranger") {
				stringstream ss;
				ss << hosts[idx].name << rand() % 10000;
				hosts[idx].name = ss.str();
			}
			string msg1 = hosts[idx].name + " has joined the chat";
			string msg2 = "Welcome to the chatroom, " + hosts[idx].name + "!\n" +
				"Please be courteous to others in the room (except Alan, he's a dick).\n" +
				"You may leave at any time by typing 'exit' or 'quit'.\n";
			for (int i = 0; i < MAX_HOSTS; i++) {
				if (hosts[i].sock) {
					if (i != idx)
						NetworkManager::GetInstance()->SendMessage(msg1, hosts[i].sock);
					else
						NetworkManager::GetInstance()->SendMessage(msg2, hosts[i].sock);
				}
			}
		} else if (data[TYPE] == QUIT) {
			RemoveClient(idx);
		} else {
			string msg = hosts[idx].name + ": " + string(&data[1]);
			for (int i = 0; i < MAX_HOSTS; i++)
				if (hosts[i].sock && i != idx)
					NetworkManager::GetInstance()->SendMessage(msg, hosts[i].sock);
		}
	}
}

void Server::AddClient()
{
	TCPsocket newsock;
	int idx;
	newsock = SDLNet_TCP_Accept(serversocket);
	if (newsock == NULL) {
		printf("Invalid new client socket\n");
		return;
	}
	/* Look for unconnected slot */
	for (idx = 0; idx < MAX_HOSTS; idx++) {
		if (!hosts[idx].sock)
			break;
	}

	if (idx == MAX_HOSTS) {
		/* No room */
		SDLNet_TCP_Send(newsock, "Goodbye", 8);
		SDLNet_TCP_Close(newsock);
	} else {
		/* Add socket */
		hosts[idx].sock = newsock;
		hosts[idx].addr = *SDLNet_TCP_GetPeerAddress(newsock);
		SDLNet_TCP_AddSocket(socketset, hosts[idx].sock);
	}
}

void Server::RemoveClient(int idx)
{
	string msg = hosts[idx].name + " left the chatroom";

	/* Notify all clients */
	for (int i = 0; i < MAX_HOSTS; i++) {
		if (hosts[i].sock) {
			if (i != idx)
				NetworkManager::GetInstance()->SendMessage(msg, hosts[i].sock);
			else
				NetworkManager::GetInstance()->SendMessage(leaveMessage, hosts[idx].sock);
		}
	}

	hosts[idx].name = "";
	hosts[idx].sock = NULL;
	SDLNet_TCP_DelSocket(socketset, hosts[idx].sock);
	SDLNet_TCP_Close(hosts[idx].sock);
}

string Server::getHostname()
{
	return hostname;
}

void Server::Stop()
{
	running = false;
	SDL_WaitThread(thread, NULL);
}
