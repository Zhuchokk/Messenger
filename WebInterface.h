#pragma once

#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include<windef.h>


#define LOW_VERSION 2
#define HIGH_VERSION 2.2
#define IP "127.0.0.1"
#define PORT 1234
#define BUFF_SIZE 256 //symbols
#define SEND_TIMEOUT 100 // millsec
#define CLIENT_TIMEOUT 110 // millsec
#define NAME_LIMIT 6 // < BUFF_SIZE
#define SOCKET_DISCONNECTED 0


bool CheckVersion();

SOCKET& CreateSocket();

void EndString(char* text, int len);

void PrintString(char* text, int len);