#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include <vector>
#include<windef.h>
#include"WebInterface.h"

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

int main() {
	CheckVersion();
	SOCKET& ClientSock = CreateSocket();

	sockaddr_in servInfo;

	in_addr ip_to_num;
	int erStat = inet_pton(AF_INET, IP, &ip_to_num);
	if (erStat < 0) {
		cout << "Error in IP translation to special numeric format" << endl;
		return 1;
	}

	ZeroMemory(&servInfo, sizeof(servInfo));

	servInfo.sin_family = AF_INET;
	servInfo.sin_addr = ip_to_num;	  // Server's IPv4 after inet_pton() function
	servInfo.sin_port = htons(1234);

	erStat = connect(ClientSock, (sockaddr*)&servInfo, sizeof(servInfo));

	if (erStat != 0) {
		cout << "Connection to Server is FAILED. Error # " << WSAGetLastError() << endl;
		closesocket(ClientSock);
		WSACleanup();
		return 1;
	}
	else
		cout << "Connection established SUCCESSFULLY. Ready to send a message to Server"<< endl;

	vector <char> servBuff(BUFF_SIZE), clientBuff(BUFF_SIZE);							
	short packet_size = 0;

	while (true) {

		cout << "Your (Client) message to Server: ";
		fgets(clientBuff.data(), clientBuff.size(), stdin);

		// Check whether client like to stop chatting 
		if (clientBuff[0] == 'x' && clientBuff[1] == 'x' && clientBuff[2] == 'x') {
			shutdown(ClientSock, SD_BOTH);
			closesocket(ClientSock);
			WSACleanup();
			return 0;
		}

		packet_size = send(ClientSock, clientBuff.data(), clientBuff.size(), 0);

		if (packet_size == SOCKET_ERROR) {
			cout << "Can't send message to Server. Error # " << WSAGetLastError() << endl;
			closesocket(ClientSock);
			WSACleanup();
			return 1;
		}

		packet_size = recv(ClientSock, servBuff.data(), servBuff.size(), 0);

		if (packet_size == SOCKET_ERROR) {
			cout << "Can't receive message from Server. Error # " << WSAGetLastError() << endl;
			closesocket(ClientSock);
			WSACleanup();
			return 1;
		}
		else
			cout << "Server message: " << servBuff.data() << endl;

	}

	closesocket(ClientSock);
	WSACleanup();

	return 0;
}