#pragma once
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include <vector>
#include<windef.h>
//#include"../WebInterface.h"
#include<thread>
#include <queue>
#include<chrono>
using namespace std;

class Server {

private:
	queue<vector<char>> mes_to_send;
	vector<SOCKET> clients;
	vector<vector<char>> names;

	void RecieveData(SOCKET client);
	void SendData();
public:
	int Work();
};
