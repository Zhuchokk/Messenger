#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include <vector>
#include<windef.h>
#include"../WebInterface.h"
#include<thread>
#include <queue>
#include<chrono>


#pragma comment(lib, "Ws2_32.lib")

using namespace std;

queue<vector<char>> mes_to_send;
vector<SOCKET> clients;

void RecieveData(SOCKET client) {
	vector <char> Buff(BUFF_SIZE);
	while (true) {
		short packet_size = recv(client, Buff.data(), Buff.size(), 0);
		if (packet_size != SOCKET_ERROR) {
			mes_to_send.push(Buff);
			PrintString(Buff.data(), Buff.size());
		}
			
	}
	
}
void SendData() {
	while (true) {
		if (!mes_to_send.empty()) {
			for (int i = 0; i < clients.size(); i++) {
				short packet_size = send(clients[i], mes_to_send.front().data(), mes_to_send.front().size(), 0);
			}
			mes_to_send.pop();
		}
		this_thread::sleep_for(chrono::milliseconds(SEND_TIMEOUT));
	}
}

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

	thread sending(SendData);
	sending.detach();

	while (true) {
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

		SOCKET ClientConn = accept(ServSock, (sockaddr*)&clientInfo, &clientInfo_size);
		if (ClientConn != INVALID_SOCKET) {
			thread receiving(RecieveData, ClientConn);
			receiving.detach();
			clients.push_back(ClientConn);
		}
			
	}

	
}