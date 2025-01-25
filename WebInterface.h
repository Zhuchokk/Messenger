#pragma once

#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include<windef.h>


#define LOW_VERSION 2
#define HIGH_VERSION 2.2
#define IP "127.0.0.1"
#define PORT 1234
#define BUFF_SIZE 10

bool CheckVersion();

SOCKET& CreateSocket();