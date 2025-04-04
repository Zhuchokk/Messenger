#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include <vector>
#include<windef.h>
#include"WebInterface.h"
#include<thread>
#include"Crypto.h"
#include"Client.h"

#pragma comment(lib, "Ws2_32.lib")

using namespace std;


void Client::RecieveData(SOCKET self) {
	vector <char> Buff(BUFF_SIZE);
	

	while (true) {
		short packet_size = recv(self, Buff.data(), Buff.size(), 0);
		if (packet_size != SOCKET_ERROR) {
			if (Buff[0] == 'S' && Buff[1] == 'M') { //Server messege
				vector<char> name;
				for (int i = 3; i < Buff.size(); i++) {
					if (Buff[i] != ' ' && Buff[i] != '\0')
						name.push_back(Buff[i]);
					else
						break;
				}
				if (Buff[2] == 'C') { //connection
					cout << "\nNew user: ";
					PrintString(name.data(), name.size());
					cout << endl;
					available_users.push_back(name);
				}
				else if(Buff[2] == 'D') { //disconnection
					cout << "\nUser ";
					PrintString(name.data(), name.size());
					cout << " diconnected" << endl;
					available_users.erase(find(available_users.begin(), available_users.end(), name));
				}
				
			}
			else {
				vector<char> txt = Crypto::retranslation(Buff, key);
				PrintString(txt.data(), txt.size());
			}	
		}
	}
}

int Client::Start() {
	int p, q, crypt_len = 0;
	vector<char> myname(NAME_LIMIT);

	CheckVersion();
	ClientSock = CreateSocket();

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
		cout << "Connection established SUCCESSFULLY. Ready to send a message to Server" << endl;

	//Getting keys
	cout << "The system is protected using RSA algorithm. Please enter two crypto keys (two prime numbers): ";
	while (true) {
		cin >> p >> q;

		if (cin.fail()) {
			cout << "Invalid input, please enter integers: ";
			cin.clear();
			cin.ignore(10000, '\n');
		}
		else if (!Crypto::IsGenPos(p, q)) {
			cout << "Incorrect values, try again: ";
		}
		else {
			break;
		}
	}
	key = Crypto::key_generation(p, q);
	int check = key.second;
	while (check != 0) {
		check /= 10;
		crypt_len++;
	}
	crypt_len++; // because of using '-' as separator
	getchar();

	//Getting the name of user, then send it
	cout << "Enter your name: ";
	int namelong = 0;
	for (int i = 0; i < NAME_LIMIT; i++) {
		if (cin.peek() == '\n') {
			myname[i] = '\0';
			if (namelong == 0) {
				namelong = i;
			}
			if (i == 0) {
				cout << "Ok. If you don't know your name, I will give you the new one: Idiot";
				myname = { 'I', 'd', 'i', 'o', 't' };
			}
			break;
		}
		else {
			myname[i] = getchar();
		}
	}
	getchar();

	send(ClientSock, myname.data(), myname.size(), 0);
}

int Client::Work() {
	thread recieving(&Client::RecieveData, this, ClientSock);
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

		bool first = 1;

		//translating and sending username with ' --> '
		int endname = 0;
		for (int i = 0; i < myname.size(); i++) {
			clientBuff[i] = myname[i];
		}
		clientBuff[namelong] = ' ';
		clientBuff[namelong + 1] = '-';
		clientBuff[namelong + 2] = '-';
		clientBuff[namelong + 3] = '>';
		clientBuff[namelong + 4] = ' ';
		clientBuff[namelong + 5] = '\0';

		encrypted = Crypto::translation(clientBuff, key);

		//Adding for encrypted text the recipient, separated by : in both sides
		encrypted.push_back(':'); //separator
		for (int i = 0; i < recipient.size(); i++) {
			if (recipient[i] != '\0') {
				encrypted.push_back(recipient[i]);
			}
		}
		encrypted.push_back(':'); //separator

		//Send
		packet_size = send(ClientSock, encrypted.data(), encrypted.size(), 0);
		this_thread::sleep_for(chrono::milliseconds(CLIENT_TIMEOUT));
		if (packet_size == SOCKET_ERROR) {
			cout << "Can't send message to Server. Error # " << WSAGetLastError() << endl;
			closesocket(ClientSock);
			WSACleanup();
			return 1;
		}

		//Getting a message from stdin and translating
		while (count(clientBuff.begin(), clientBuff.end(), '\n') == 0 || first) {
			fgets(clientBuff.data(), clientBuff.size(), stdin);
			// Check whether client like to stop chatting 
			if (clientBuff[0] == '/') {
				cout << "Finishing session...";
				shutdown(ClientSock, SD_BOTH);
				closesocket(ClientSock);
				WSACleanup();
				return 0;
			}

			encrypted = Crypto::translation(clientBuff, key);

			//Adding for encrypted text the recipient, separated by : in both sides
			encrypted.push_back(':'); //separator
			for (int i = 0; i < recipient.size(); i++) {
				if (recipient[i] != '\0') {
					encrypted.push_back(recipient[i]);
				}
			}
			encrypted.push_back(':'); //separator


			//Send
			packet_size = send(ClientSock, encrypted.data(), encrypted.size(), 0);
			this_thread::sleep_for(chrono::milliseconds(CLIENT_TIMEOUT));
			if (packet_size == SOCKET_ERROR) {
				cout << "Can't send message to Server. Error # " << WSAGetLastError() << endl;
				closesocket(ClientSock);
				WSACleanup();
				return 1;
			}
			first = 0;
		}
	}

	closesocket(ClientSock);
	WSACleanup();

	return 0;
}


int main() {
	Client c;
	c.Start();
	c.Work();
}