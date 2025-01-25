#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include <vector>
#include<windef.h>
#include"../WebInterface.h"


#pragma comment(lib, "Ws2_32.lib")

using namespace std;

int main() {
	CheckVersion();
	SOCKET& ServSock = CreateSocket();

	in_addr ip_to_num;
	int erStat = inet_pton(AF_INET, IP, &ip_to_num);
	if (erStat < 0) {
		cout << "Error in IP translation to special numeric format" << endl;
		return 1;
	}

	sockaddr_in servInfo;
	ZeroMemory(&servInfo, sizeof(servInfo));

	servInfo.sin_family = AF_INET;
	servInfo.sin_addr = ip_to_num;
	servInfo.sin_port = htons(PORT);

	erStat = bind(ServSock, (sockaddr*)&servInfo, sizeof(servInfo));
	if (erStat != 0) {
		cout << "Error Socket binding to server info. Error # " << WSAGetLastError() << endl;
		closesocket(ServSock);
		WSACleanup();
		return 1;
	}
	else
		cout << "Binding socket to Server info is OK" << endl;

	erStat = listen(ServSock, SOMAXCONN);

	if (erStat != 0) {
		cout << "Can't start to listen to. Error # " << WSAGetLastError() << endl;
		closesocket(ServSock);
		WSACleanup();
		return 1;
	}
	else {
		cout << "Listening..." << endl;
	}

	sockaddr_in clientInfo;

	ZeroMemory(&clientInfo, sizeof(clientInfo));

	int clientInfo_size = sizeof(clientInfo);

	SOCKET ClientConn1 = accept(ServSock, (sockaddr*)&clientInfo, &clientInfo_size);

	if (ClientConn1 == INVALID_SOCKET) {
		cout << "Client detected, but can't connect to a client. Error # " << WSAGetLastError() << endl;
		closesocket(ServSock);
		closesocket(ClientConn1);
		WSACleanup();
		return 1;
	}
	else
		cout << "Connection to a client established successfully" << endl;

	erStat = listen(ServSock, SOMAXCONN);
	SOCKET ClientConn2 = accept(ServSock, (sockaddr*)&clientInfo, &clientInfo_size);

	vector <char> servBuff(BUFF_SIZE), clientBuff(BUFF_SIZE);
	short packet_size = 0;

	while (true) {
		packet_size = recv(ClientConn1, servBuff.data(), servBuff.size(), 0);
		packet_size = send(ClientConn2, servBuff.data(), servBuff.size(), 0);

		packet_size = recv(ClientConn2, servBuff.data(), servBuff.size(), 0);
		packet_size = send(ClientConn1, servBuff.data(), servBuff.size(), 0);


		if (packet_size == SOCKET_ERROR) {
			cout << "Can't send message to Client. Error # " << WSAGetLastError() << endl;
			closesocket(ServSock);
			closesocket(ClientConn1);
			WSACleanup();
			return 1;
		}

	}
}