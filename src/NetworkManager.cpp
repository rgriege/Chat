#include "NetworkManager.h"

#include <sstream>
#include <iostream>

#ifdef _WIN32
#include <WinSock.h>
#undef SendMessage
#endif

using namespace std;

NetworkManager* NetworkManager::instance = NULL;

NetworkManager::NetworkManager()
{
	/* Initialize SDLnet */
	if (SDLNet_Init() < 0) {
		cout << "Couldn't initialize net:" << SDLNet_GetError() << endl;
		SDL_Quit();
		exit(EXIT_FAILURE);
	}

	/* Allocate the socket set */
	socketset = SDLNet_AllocSocketSet(1);
	if (socketset == NULL) {
		cout << "Couldn't create socket set" << SDLNet_GetError();
		exit(EXIT_FAILURE);
	}
}

NetworkManager* NetworkManager::GetInstance()
{
	if (!instance)
		instance = new NetworkManager();
	return instance;
}

std::string NetworkManager::GetHostname()
{
	char compname[32]; // = (char*)malloc(len);
#ifdef _WIN32
	WSADATA wsa_data;
	/* Load Winsock 2.0 DLL */
	if (WSAStartup(MAKEWORD(2, 0), &wsa_data) != 0) {
		cerr << "WSAStartup() failed" << WSAGetLastError() << endl;
		exit (EXIT_FAILURE);
	}
	if(gethostname(compname, 31))
		cout << "error: " << WSAGetLastError() << endl;
	else
		cout << "ifdef hostname: " << compname << endl;
	WSACleanup(); /* Cleanup Winsock */
#elif __linux__
	gethostname(compname, len);
#endif
	std::string result(compname);
	//delete[] compname;
	return result;
}

std::string NetworkManager::GetIP()
{
	const char* compname = GetHostname().c_str();

	if (SDLNet_ResolveHost(&ip, compname, GAME_PORT) < 0) {
		cout << "Couldn't get IP:" << SDLNet_GetError() << endl;
		return NULL;
	}

	return printIP(ip);
}

std::string printIP(IPaddress ip) {
	std::stringstream addr_ss;
	Uint32 addr = ip.host;
	for (int i = 0; i < 4; i++) {
		addr_ss << (ip.host >> 8*i) % 256;
		if (i < 3) addr_ss << ".";
	}
	addr_ss << ":" << ip.port;
	return addr_ss.str();
}

void NetworkManager::ConnectToServer(string host)
{
	IPaddress ip;
	/* Resolve the host we are connecting to */
	if (SDLNet_ResolveHost(&ip, host.c_str(), GAME_PORT) < 0) {
		fprintf(stderr, "SDLNet_ResolveHost %s: %s\n", host.c_str(), SDLNet_GetError());
		exit(EXIT_FAILURE);
	}

	if (ip.host == INADDR_NONE) {
		printf("Couldn't resolve hostname\n");
	}

	/* Open a connection with the IP provided (listen on the host's port) */
	if (!(serversocket = SDLNet_TCP_Open(&ip))) {
		fprintf(stderr, "SDLNet_TCP_Open: %s\n", SDLNet_GetError());
		exit(EXIT_FAILURE);
	}

	if (SDLNet_TCP_AddSocket(socketset, serversocket) == -1) {
		cout << "SDLNet_TCP_AddSocket: " << SDLNet_GetError();
		exit(EXIT_FAILURE);
	}
}

void NetworkManager::SendMessage(string msg, TCPsocket sd)
{
	if (sd)
		SDLNet_TCP_Send(sd, msg.c_str(), msg.length()+1);
	else
		SDLNet_TCP_Send(serversocket, msg.c_str(), msg.length()+1);
}

void NetworkManager::SendMessage(char* msg, TCPsocket sd)
{
	if (sd)
		SDLNet_TCP_Send(sd, msg, strlen(msg)+1);
	else
		SDLNet_TCP_Send(serversocket, msg, strlen(msg)+1);
}

bool NetworkManager::CheckSocket()
{
	if (SDLNet_CheckSockets(socketset, 0) >= 0) {
		if (SDLNet_SocketReady(serversocket))
			return true;
	} else {
		cout << "SDLNet_TCP_CheckSockets: " << SDLNet_GetError();
		exit(EXIT_FAILURE);
	}
	return false;
}

std::string NetworkManager::ReceiveMessage()
{
	char msg[1024];
	int len = SDLNet_TCP_Recv(serversocket, msg, 1023);
	if (len <= 0) {
		SDLNet_TCP_Close(serversocket);
		fprintf(stderr, "SDLNet_TCP_RECV: %s\n", SDLNet_GetError());
		exit(EXIT_FAILURE);
	}
	msg[len] = 0;
	return std::string(msg);
}

void NetworkManager::Close()
{
	SDLNet_TCP_Close(serversocket);
	SDLNet_Quit();
}
