#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include <vector>
#include<windef.h>
#include"WebInterface.h"
#include<thread>
#include"Crypto.h"

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

pair<pair<int, int>, int> key;

void RecieveData(SOCKET self) {
	vector <char> Buff(BUFF_SIZE);
	

	while (true) {
		short packet_size = recv(self, Buff.data(), Buff.size(), 0);
		if (packet_size != SOCKET_ERROR) {
			vector<char> txt = retranslation(Buff, key);
			PrintString(txt.data(), txt.size());
		}

	}

}

int main() {
	int p, q, crypt_len = 0;

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

	cout << "The system is protected using RSA algorithm. Please enter 2 crypto keys:";
	cin >> p >> q;
	while (!IsGenPos(p, q)) {
		cout << "Incorrect values, try again:";
		cin >> p >> q;
	}
	key = key_generation(p, q);
	int check = key.second;
	while (check != 0) {
		check /= 10;
		crypt_len++;
	}
	crypt_len++; // because of using '-' as separator
	
	thread recieving(RecieveData, ClientSock);
	recieving.detach();

	vector <char> clientBuff(BUFF_SIZE / crypt_len), encrypted(BUFF_SIZE);							
	short packet_size = 0;


	while (true) {
		vector<char> encrypted(BUFF_SIZE);
		fgets(clientBuff.data(), clientBuff.size(), stdin);
		/*EndString(clientBuff.data(), clientBuff.size());*/

		//cout << "for encrypt:\n";
		//PrintString(clientBuff.data(), clientBuff.size());

		encrypted = translation(clientBuff, key);
		//PrintString(retranslation(encrypted, key).data(), BUFF_SIZE);

		// Check whether client like to stop chatting 
		/*if (clientBuff[0] == 'x' && clientBuff[1] == 'x' && clientBuff[2] == 'x') {
			shutdown(ClientSock, SD_BOTH);
			closesocket(ClientSock);
			WSACleanup();
			return 0;
		}*/

		packet_size = send(ClientSock, encrypted.data(), encrypted.size(), 0);

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