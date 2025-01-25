#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include <vector>
#include<windef.h>
#include"WebInterface.h"
#include<thread>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

void RecieveData(SOCKET self) {
	vector <char> Buff(BUFF_SIZE);


	while (true) {
		short packet_size = recv(self, Buff.data(), Buff.size(), 0);
		if (packet_size != SOCKET_ERROR) {
			PrintString(Buff.data(), Buff.size());
		}

	}

}


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
	servInfo.sin_port = htons(PORT);

	erStat = connect(ClientSock, (sockaddr*)&servInfo, sizeof(servInfo));

	if (erStat != 0) {
		cout << "Connection to Server is FAILED. Error # " << WSAGetLastError() << endl;
		closesocket(ClientSock);
		WSACleanup();
		return 1;
	}
	else
		cout << "Connection established SUCCESSFULLY. Ready to send a message to Server"<< endl;

	thread recieving(RecieveData, ClientSock);
	recieving.detach();

	vector <char> clientBuff(BUFF_SIZE);							
	short packet_size = 0;

	while (true) {

		cout << "Your (Client) message to Server: ";
		fgets(clientBuff.data(), clientBuff.size(), stdin);
		EndString(clientBuff.data(), clientBuff.size());

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

		

	}

	closesocket(ClientSock);
	WSACleanup();

	return 0;
}