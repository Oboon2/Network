#define _WINSOCK_DEPRECATED_NO_WARNINGS 

#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

int main()
{
	WSADATA wsaData;
	SOCKET hServerSocket;
	SOCKADDR_IN ServerAddress;

	char Buffer[1024 * 8] = { 0, };
	char Message[1024 * 8] = { 0, };
	int MessageLength;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WinSocket Error %d\n", GetLastError());
		exit(-1);
	}

	hServerSocket = socket(PF_INET, SOCK_STREAM, 0);
	if (hServerSocket == INVALID_SOCKET)
	{
		printf("socket() Error %d\n", GetLastError());
		exit(-1);
	}

	memset(&ServerAddress, 0, sizeof(SOCKADDR_IN));
	ServerAddress.sin_family = PF_INET;
	ServerAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
	ServerAddress.sin_port = htons(4000);

	if (connect(hServerSocket, (SOCKADDR*)&ServerAddress, sizeof(ServerAddress)) == SOCKET_ERROR)
	{
		printf("connect Error %d\n", GetLastError());
		exit(-1);
	}

	while (true)
	{
		cout << "Message :";
		cin >> Message;

		MessageLength = send(hServerSocket, Message, strlen(Message), 0);
		if (MessageLength == -1)
		{
			printf("send() Error %d\n", GetLastError());
			exit(-1);
		}
		if (strcmp(Message, "end") == 0)
		{
			send(hServerSocket, Message, strlen(Message), 0);
			closesocket(hServerSocket);
			break;
		}
		else
		{
			int recvLength = recv(hServerSocket, Buffer, MessageLength, 0);
			if (recvLength == -1)
			{
				printf("recv() error %d\n", GetLastError());
				exit(-1);
			}
			cout << "Message from server : " << Buffer << endl;
			memset(Buffer, 0, sizeof(Buffer));
		}
	}

	WSACleanup();
	return 0;

}