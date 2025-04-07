#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include <vector>
#include<windef.h>
#include"WebInterface.h"
#include <thread>
#include"Crypto.h"
#include"Client.h"

#pragma comment(lib, "Ws2_32.lib")

using namespace std;


void Client::RecieveData(SOCKET self) {
	vector <char> Buff(BUFF_SIZE);
	

	while (true) {
		short packet_size = recv(self, Buff.data(), Buff.size(), 0);
		if (packet_size == SOCKET_DISCONNECTED) {
			cout << endl << "You are disconnected from server." << endl;
			return;
		}
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
	int p, q;
	myname.resize(NAME_LIMIT);
	CheckVersion();
	ClientSock = CreateSocket();

	sockaddr_in servInfo;

	in_addr ip_to_num;
	int erStat = inet_pton(AF_INET, IP, &ip_to_num);
	if (erStat < 0) {
		Exception Temp("Error in IP translation to special numeric format");
		cout << Temp.text << endl;
		throw Temp;

		//cout << "Error in IP translation to special numeric format" << endl;
		return 1;
	}

	ZeroMemory(&servInfo, sizeof(servInfo));

	servInfo.sin_family = AF_INET;
	servInfo.sin_addr = ip_to_num;	  // Server's IPv4 after inet_pton() function
	servInfo.sin_port = htons(PORT);

	erStat = connect(ClientSock, (sockaddr*)&servInfo, sizeof(servInfo));

	if (erStat != 0) {
		Exception Temp("Connection to Server is FAILED. Error # ");
		cout << Temp.text << endl;
		throw Temp;

		//cout << "Connection to Server is FAILED. Error # " << WSAGetLastError() << endl;
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
	namelong = 0;
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
			Exception Temp("Can't send message to Server. Error # ");
			cout << Temp.text << endl;
			throw Temp;
			
			//cout << "Can't send message to Server. Error # " << WSAGetLastError() << endl;
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
				Exception Temp("Can't send message to Server. Error # ");
				cout << Temp.text << endl;
				throw Temp;

				//cout << "Can't send message to Server. Error # " << WSAGetLastError() << endl;
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

bool Administrator::IsSubVector(vector<char>& main, const vector<char>& sub) {
	for (int i = 0; i < sub.size(); i++) {
		if (main[i] != '\0' && sub[i] != '\0' && main[i] != sub[i]) {
			return false;
		}
	}
	return true;
}

int Administrator::Work() {
	thread recieving(&Administrator::RecieveData, this, ClientSock);
	recieving.detach();

	vector <char> clientBuff((BUFF_SIZE - NAME_LIMIT - 2) / crypt_len);
	short packet_size = 0;

	while (true) {
		if (cin.peek() == '/') {
			getchar(); //skip /
			vector<char> command(20);
			for (int i = 0; i < 20; i++) {
				if (cin.peek() == '\n') {
					command[i] = '\0';
					break;
				}
				else {
					command[i] = getchar();
				}
			}
			getchar(); //skip \n
			if (IsSubVector(command, { 'l', 'i', 's', 't', '\0' })) {
				for (int i = 0; i < available_users.size(); ++i) {
					for (char c : available_users[i]) {
						cout << c;
					}
					cout << endl;
				}
			}
			else if (IsSubVector(command, { 'd', 'e', 'l', ' ' })) {
				vector<char> user_del;
				for (int i = 4; i < command.size(); i++) {
					if (command[i] == '\0') {
						break;
					}
					else {
						user_del.push_back(command[i]);
					}
				}
				if (find(available_users.begin(), available_users.end(), user_del) != available_users.end())
					send(ClientSock, command.data(), command.size(), 0);
				else
					cout << "User doesn't exist" << endl;
			}
			else {
				cout << "Unknown command" << endl;
			}
			continue;
		}
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
			Exception Temp("Can't send message to Server. Error # ");
			cout << Temp.text << endl;
			throw Temp;

			//cout << "Can't send message to Server. Error # " << WSAGetLastError() << endl;
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
				Exception Temp("Can't send message to Server. Error # ");
				cout << Temp.text << endl;
				throw Temp;

				//cout << "Can't send message to Server. Error # " << WSAGetLastError() << endl;
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
	cout << "Are you an administrator? (y/n)" << endl;
	char a; cin >> a;
	if (a == 'y') {
		Administrator admin;
		admin.Start();
		admin.Work();
	}
	else {
		Client c;
		c.Start();
		c.Work();
	}
	
}