#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include<windef.h>
#include"WebInterface.h"
#include<vector>

#pragma comment(lib, "Ws2_32.lib")


using namespace std;

bool CheckVersion() {
	WSADATA wsData;
	int erStat = WSAStartup(MAKEWORD(LOW_VERSION, HIGH_VERSION), &wsData);

	if (erStat != 0) {
		cout << "Error WinSock version initializaion #";
		cout << WSAGetLastError();
		return 0;
	}
	else {
		cout << "WinSock initialization is OK" << endl;
		return 1;
	}
}

SOCKET& CreateSocket() {
	SOCKET* Sock = new SOCKET;
	*Sock = socket(AF_INET, SOCK_STREAM, 0);

	if (*Sock == INVALID_SOCKET) {
		cout << "Error initialization socket # " << WSAGetLastError() << endl;
		closesocket(*Sock);
		WSACleanup();
		*Sock = NULL;
	}
	else
		cout << "Server socket initialization is OK" << endl;
	return *Sock;
}

void EndString(char* text, int len) {
	for (int i = 0; i < len; i++) {
		if (text[i] == ' ') {
			text[i] = '\0';
			return;
		}
	}
}

void PrintString(char* text, int len) {
	int i = 0;
	while (text[i] != '\0' && i < len) {
		cout << text[i++];
	}
}