#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <iostream>
#include <time.h>

#pragma comment(lib, "ws2_32.lib")
using namespace std;

DWORD WINAPI ThreadRun1(LPVOID Arg);
DWORD WINAPI ThreadRun2(LPVOID Arg);

long long num = 0;

HANDLE hMutex;

#define ThreadNumber 20
#define PORT 4000

int main()
{
	WSADATA wsaData;
	SOCKET ClientSocketHandle;
	SOCKADDR_IN ServerAddress;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("winsock error\n");
		exit(-1);
	}

	ClientSocketHandle = socket(PF_INET, SOCK_DGRAM, 0); // UDP
	if (ClientSocketHandle == INVALID_SOCKET)
	{
		printf("socket error\n");
		exit(-1);
	}

	memset(&ServerAddress, 0, sizeof(ServerAddress));
	ServerAddress.sin_family = PF_INET;
	ServerAddress.sin_addr.s_addr = inet_addr("192.168.0.173");
	ServerAddress.sin_port = htons(PORT);

	char Message[8192] = { 0, };
	char Buffer[8192] = { 0, };
	int MessageLength = 0;


	while (true)
	{
		cout << "Message : ";
		cin >> Message;

		MessageLength = strlen(Message);

		int SendtoLength = sendto(ClientSocketHandle, Message, MessageLength, 0, (SOCKADDR*)&ServerAddress, sizeof(ServerAddress));
		if (SendtoLength == -1)
		{
			printf("sendto error\n");
			exit(-1);
		}
		int ServerAddressSize = sizeof(ServerAddress);
		memset(Buffer, 0, sizeof(Buffer));

		int RecvLength = recvfrom(ClientSocketHandle, Buffer, sizeof(Buffer) - 1, 0, (SOCKADDR*)&ServerAddress, &ServerAddressSize);

		if (RecvLength == -1)
		{
			printf("recvfrom error\n");
			exit(-1);
		}
		printf("Server from : %s\n", Buffer);
			
		if(strcmp(Buffer, "end") == 0)
		{
			break;
		}
		time_t start;
		time_t end;
		HANDLE* ThreadHandle;
		// ThreadHandle = (HANDLE*)malloc(sizeof(ThreadHandle) * ThreadNumber);
		ThreadHandle = new HANDLE[ThreadNumber];


		hMutex = CreateMutex(NULL, FALSE, NULL);


		start = time(NULL);
		cout << "first : " << num << endl;

		for (int i = 0; i < ThreadNumber; ++i)
		{
			if (i % 2 == 0)
			{
				ThreadHandle[i] = CreateThread(NULL, 0, ThreadRun1, NULL, 0, NULL);
			}
			else
			{
				ThreadHandle[i] = CreateThread(NULL, 0, ThreadRun2, NULL, 0, NULL);
			}
		}
		WaitForMultipleObjects(ThreadNumber, ThreadHandle, TRUE, INFINITE);
		end = time(NULL);

		cout << "end : " << num << endl;
		cout << "time : " << end - start << endl;

		CloseHandle(hMutex);

		for (int i = 0; i < ThreadNumber; ++i)
		{
			CloseHandle(ThreadHandle[i]);
		}
		delete[] ThreadHandle;
	}
	closesocket(ClientSocketHandle);
	WSACleanup();

	return 0;
}

DWORD WINAPI ThreadRun1(LPVOID Arg)
{
	int sock = *((int*)Arg);
	char Buffer[8192] = { 0, };
	char Message[8192] = { 0, };
	int MessageLength;



	while (true)
	{
		cout << "Message : ";
		cin >> Message;


		MessageLength = send(sock, Message, strlen(Message), 0);
		if (MessageLength == -1)
		{
			printf("send() error %d\n", GetLastError());
			exit(-1);
		}
	}
	WSACleanup();
	return 0;
}

DWORD WINAPI ThreadRun2(LPVOID Arg)
{
	int sock = *((int*)Arg);
	char Buffer[8192] = { 0, };
	char Message[8192] = { 0, };



	while (true)
	{
		if (strcmp(Message, "end") == 0)
		{
			//우아한 종료용
			send(sock, Message, strlen(Message), 0);
			closesocket(sock);
			break;
		}
		else
		{
			memset(Buffer, 0, sizeof(Buffer));
			int recvLength = recv(sock, Buffer, sizeof(Buffer) - 1, 0);
			if (recvLength == -1)
			{
				printf("recv() error %d\n", GetLastError());
				exit(-1);
			}
			cout << "Message from server : " << Buffer << endl;

		}
	}
	WSACleanup();
	return 0;
}
