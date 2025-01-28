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
vector<vector<char>> names;

void RecieveData(SOCKET client) {
	bool greeting = 0;
	while (true) {
		vector <char> Buff(BUFF_SIZE);
		short packet_size = recv(client, Buff.data(), Buff.size(), 0);

		if (packet_size == SOCKET_DISCONNECTED) {
			int index = find(clients.begin(), clients.end(), client) - clients.begin();

			cout << "User ";
			PrintString(names[index].data(), names[index].size());
			cout << " disconnected" << endl;

			vector<char> user_deleted = names[index];

			user_deleted.insert(user_deleted.begin(), 'D');
			user_deleted.insert(user_deleted.begin(), 'M');
			user_deleted.insert(user_deleted.begin(), 'S');
			user_deleted.push_back(':');
			user_deleted.push_back(':');
			mes_to_send.push(user_deleted);

			names.erase(names.begin() + index);
			return;
		}
			
		//first greeting, the user sending his name
		if (!greeting) {
			clients.push_back(client);
			int rs = 0;
			for (int i = 0; i < Buff.size(); i++) {
				if (Buff[i] != ' ' && Buff[i] != '\0') {
					rs++;
				}
				else {
					break;
				}
			}
			Buff.resize(rs);
			//checking wether the name is duplicate
			if (count(names.begin(), names.end(), Buff) != 0) {
				cout << "There is a duplicate of user ";
				PrintString(Buff.data(), Buff.size());
				cout << endl;
				closesocket(client);
				return;
			}
			names.push_back(Buff);
			cout << "User ";
			PrintString(Buff.data(), Buff.size());
			cout << " connected" << endl;
			//Sending message about new user to all
			Buff.insert(Buff.begin(), 'C');
			Buff.insert(Buff.begin(), 'M');
			Buff.insert(Buff.begin(), 'S');
			Buff.push_back(':');
			Buff.push_back(':');
			mes_to_send.push(Buff);
			greeting = 1;
		}
		//ordinary recieving
		else {
			if (packet_size != SOCKET_ERROR) {
				mes_to_send.push(Buff);
				PrintString(Buff.data(), Buff.size());
				cout << endl;
			}
		}
		
			
	}
	
}
void SendData() {
	while (true) {
		if (!mes_to_send.empty()) {
			//Separating a name from a text
			auto start = find(mes_to_send.front().begin(), mes_to_send.front().end(), ':');
			vector<char> recipient_name;
			start++;
			while (start != mes_to_send.front().end() && *start != ':') {
				recipient_name.push_back(*start);
				start++;
				
			}
			mes_to_send.front().resize(start - mes_to_send.front().begin() -1 - recipient_name.size());

			if (recipient_name.size() != 0) {
				//sending only to certain names
				for (int i = 0; i < names.size(); i++) {
					if (names[i] == recipient_name) {
						short packet_size = send(clients[i], mes_to_send.front().data(), mes_to_send.front().size(), 0);
					}
				}
			}
			else {
				//sending to all
				for (int i = 0; i < clients.size(); i++) {

					short packet_size = send(clients[i], mes_to_send.front().data(), mes_to_send.front().size(), 0);
				}
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
		}
			
	}

	
}