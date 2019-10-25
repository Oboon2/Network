#define _WINSOCK_DEPRECATED_NO_WARNINGS 

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <WinSock2.h>
#include "Player.h"
#include <vector>
#include "Common.h"
#include "PacketMaker.h"
#include <conio.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

vector <Player*> UserList;

CRITICAL_SECTION UserListSync;

DWORD WINAPI ReceiveThread(LPVOID Arg);
void Render();
void Gotoxy(int x, int y);

bool bIsRunning = true;

int main()
{
	WSADATA wsaData;
	SOCKET ServerSocket;
	SOCKADDR_IN ServerAddress;

	InitializeCriticalSection(&UserListSync);

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WinSock2 Error\n");
		exit(-1);
	}
	ServerSocket = socket(PF_INET, SOCK_STREAM, 0);
	if (ServerSocket == INVALID_SOCKET)
	{
		printf("socket Error\n");
		exit(-1);
	}

	memset(&ServerAddress, 0, sizeof(ServerAddress));
	ServerAddress.sin_family = PF_INET;
	ServerAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
	ServerAddress.sin_port = htons(4000);

	if (connect(ServerSocket, (SOCKADDR*)&ServerAddress, sizeof(ServerAddress)) == SOCKET_ERROR)
	{
		printf("connect Error\n");
		exit(-1);
	}

	DWORD ThreadID;
	HANDLE ReceiveThreadHandle = CreateThread(nullptr, 0, ReceiveThread, (LPVOID)&ServerSocket, 0, &ThreadID);

	char Message[1024] = { 0, };
	while (bIsRunning)
	{
		int key = getch();

		if (key == 224)
		{
			key = getch();

			EnterCriticalSection(&UserListSync);
			switch (key)
			{
			case 72:
				UserList[0]->Y--;
				break;
			case 80:
				UserList[0]->Y++;
				break;
			case 75:
				UserList[0]->X--;
				break;
			case 77:
				UserList[0]->X++;
				break;

			default:
				break;
			}
			LeaveCriticalSection(&UserListSync);

			PacketMaker PM;
			PM.MakeMovePacket(*UserList[0]);
			send(ServerSocket, PM.Packet, PM.PacketSize, 0);
		}

		Render();
	}

	DeleteCriticalSection(&UserListSync);
}

DWORD WINAPI ReceiveThread(LPVOID Arg)
{
	SOCKET ServerSocket = *(SOCKET*)Arg;

	char Buffer[1024] = { 0, };

	while (true)
	{
		memset(Buffer, 0, sizeof(Buffer));

		int recvLen = recv(ServerSocket, Buffer, 3, 0);
		if (recvLen == 0)
		{
			printf("Disconnect\n");
			exit(-1);
		}
		else if (recvLen == SOCKET_ERROR)
		{
			printf("Error Disconnect\n");
			exit(-1);
		}
		else
		{
			uint16 Temp;
			byte PacketLength;

			memcpy(&Temp, Buffer, 2);
			memcpy(&PacketLength, &Buffer[2], 1);

			MSGCode Code = (MSGCode)Temp;

			int BodyLength = 0;
			int HeaderLength = 3;
			PacketLength -= HeaderLength;
			BodyLength = PacketLength;

			while (BodyLength > 0)
			{
				int recvLen = recv(ServerSocket, &Buffer[PacketLength - BodyLength], BodyLength, 0);
				if (recvLen <= 0)
				{
					printf("Error or Disconnect\n");
					exit(-1);
				}
				else
				{
					BodyLength -= recvLen;
				}
			}

			switch (Code)
			{
				case MSGCode::Enter:
				{
					Player* CanAddPlayer = new Player();
					PacketMaker PM;
					PM.ExtractPacket(Code, Buffer, CanAddPlayer);

					EnterCriticalSection(&UserListSync);

					bool bInsert = true;
					for (auto User : UserList)
					{
						if (User->UserID == CanAddPlayer->UserID)
						{
							bInsert = false;
							break;
						}
					}
					if (bInsert)
					{
						UserList.push_back(CanAddPlayer);
					}
					else
					{
						delete CanAddPlayer;
					}

					LeaveCriticalSection(&UserListSync);
				}
				break;

				case MSGCode::Exit:
				{
					printf("Exit\n");
					EnterCriticalSection(&UserListSync);

					Player* DeleteUser = nullptr;
					Player* ExitUser = new Player();
					PacketMaker PM;
					PM.ExtractPacket(Code, Buffer, ExitUser);

					for (auto User : UserList)
					{
						if (User->UserID == ExitUser->UserID)
						{
							DeleteUser = User;
						}
					}
					UserList.erase(find(UserList.begin(), UserList.end(), DeleteUser));
					delete DeleteUser;
					delete ExitUser;
					LeaveCriticalSection(&UserListSync);
				}
				break;

				case MSGCode::Move:
				{
					Player MoveUser;
					PacketMaker PM;
					PM.ExtractPacket(Code, Buffer, &MoveUser);

					for (auto User : UserList)
					{
						if (User->UserID == MoveUser.UserID)
						{
							EnterCriticalSection(&UserListSync);
							User->X = MoveUser.X;
							User->Y = MoveUser.Y;
							LeaveCriticalSection(&UserListSync);
							break;
						}
					}
				}
				break;

				case MSGCode::Bye:
				{
					Player ByeUser;
					PacketMaker PM;
					PM.ExtractPacket(Code, Buffer, &ByeUser);
					system("cls");
					printf("Bye");
					bIsRunning = false;
					closesocket(ServerSocket);

					for (auto User : UserList)
					{
						delete User;
					}

					UserList.clear();

					return 0;
				}
			}

			Render();
		}
	}

	return 0;
}

void Render()
{
	EnterCriticalSection(&UserListSync);
	system("cls");
	Gotoxy(1, 1);
	printf("UserCount %d", UserList.size());
	for (auto User : UserList)
	{
		Gotoxy(User->X, User->Y);
		printf("%d", User->UserID);
	}
	LeaveCriticalSection(&UserListSync);
}

void Gotoxy(int x, int y)
{
	COORD Pos;
	Pos.X = x;
	Pos.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), Pos);
}
