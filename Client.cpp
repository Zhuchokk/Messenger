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
	vector<char> myname(NAME_LIMIT);

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

	//Getting keys
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
	getchar();

	//Getting the name of user, then send it
	cout << "Enter your name: ";
	for (int i = 0; i < NAME_LIMIT; i++) {
		if (cin.peek() == '\n') {
			myname[i] = '\0';
			break;
		}
		else {
			myname[i] = getchar();
		}
	}
	getchar();

	send(ClientSock, myname.data(), myname.size(), 0);
	
	

	thread recieving(RecieveData, ClientSock);
	recieving.detach();

	vector <char> clientBuff((BUFF_SIZE - NAME_LIMIT - 2) / crypt_len);							
	short packet_size = 0;


	while (true) {

		vector<char> encrypted(BUFF_SIZE), recipient(NAME_LIMIT);
		for (int i = 0; i < NAME_LIMIT; i++) {
			if (cin.peek() == ':') {
				recipient[i] = '\0';
				break;
			}
			else {
				recipient[i] = getchar();
			}
		}
		getchar(); // plug for avoiding ':' in fgets

		//Getting a message from stdin and translating
		fgets(clientBuff.data(), clientBuff.size(), stdin);
		encrypted = translation(clientBuff, key);

		//Adding for encrypted text the recipient, separated by : in both sides
		encrypted.push_back(':'); //separator
		for (int i = 0; i < recipient.size(); i++) {
			if (recipient[i] != '\0') {
				encrypted.push_back(recipient[i]);
			}
		}
		encrypted.push_back(':'); //separator

		// Check whether client like to stop chatting 
		if (clientBuff[0] == '/') {
			cout << "Finishing session...";
			shutdown(ClientSock, SD_BOTH);
			closesocket(ClientSock);
			WSACleanup();
			return 0;
		}

		//Send
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