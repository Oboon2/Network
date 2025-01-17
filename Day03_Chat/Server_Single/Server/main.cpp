#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <WinSock2.h>
#include <vector>
#include <Windows.h>

#pragma comment(lib, "ws2_32.lib")


using namespace std;

vector<SOCKET> UserList;


CRITICAL_SECTION UserListSync;

int main()
{
	WSAData wsaData;
	SOCKET ServerSocket;
	SOCKADDR_IN ServerAddress;
	InitializeCriticalSection(&UserListSync);

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("Winsock2 error\n");
		exit(-1);
	}

	ServerSocket = socket(PF_INET, SOCK_STREAM, 0);
	if (ServerSocket == INVALID_SOCKET)
	{
		printf("socket error\n");
		exit(-1);
	}

	memset(&ServerAddress, 0, sizeof(ServerAddress));
	ServerAddress.sin_family = PF_INET;
	ServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	ServerAddress.sin_port = htons(4000);

	if (bind(ServerSocket, (SOCKADDR*)&ServerAddress, sizeof(ServerAddress)) == SOCKET_ERROR)
	{
		printf("bind error\n");
		exit(-1);
	}

	if (listen(ServerSocket, 5) == SOCKET_ERROR)
	{
		printf("listen error");
		exit(-1);
	}


	SOCKADDR_IN ClientAddress;
	int ClientAddressLength = sizeof(ClientAddress);

	TIMEVAL TimeOut; // select 대기 시간
	fd_set Reads;
	fd_set CopyReads;

	FD_ZERO(&Reads);
	FD_SET(ServerSocket, &Reads);

	while (true)
	{
		CopyReads = Reads;

		TimeOut.tv_sec = 0;
		TimeOut.tv_usec = 5;

		int fdReadCount = select(0, &CopyReads, 0, 0, &TimeOut);
		if (fdReadCount == SOCKET_ERROR)
		{
			printf("select error\n");
			exit(-1);
		}
		else if (fdReadCount == 0)
		{
			// 소켓 변화가 없음
			continue;
		}
		
		// 감시하는 소켓 갯수만큼 확인
		for (u_int i = 0; i < Reads.fd_count; ++i)
		{
			// i 번째 소켓의 변화가 있음, 소켓에 정보가 들어옴
			if (FD_ISSET(Reads.fd_array[i], &CopyReads))
			{
				// 접속 요청
				if (Reads.fd_array[i] == ServerSocket)
				{
					SOCKET ConnectClientSocket = accept(ServerSocket, (SOCKADDR*)&ClientAddress, &ClientAddressLength);

					// 유저 리스트 관리
					EnterCriticalSection(&UserListSync);
					UserList.push_back(ConnectClientSocket);
					LeaveCriticalSection(&UserListSync);

					// 감시 소켓 리스트, 클라이언트 소켓 추가
					FD_SET(ConnectClientSocket, &Reads);

					printf("Client Connect\n");
				}
				else // 접속 종료, 관리하는 클라이언트의 변화가 생김
				{
					char Buffer[1024] = { 0, };
					int recvlen = recv(Reads.fd_array[i], Buffer, sizeof(Buffer) - 1, 0);

					if (recvlen <= 0)
					{
						 //error 종료거나 연결이 끊겼거나
						SOCKET DisconnectSocket = Reads.fd_array[i];
						FD_CLR(DisconnectSocket, &Reads);
						closesocket(DisconnectSocket);
						printf("Disconnect client.\n");

						EnterCriticalSection(&UserListSync);
						UserList.erase(find(UserList.begin(), UserList.end(), DisconnectSocket));
						LeaveCriticalSection(&UserListSync);
					}
					else // 클라이언트한테서 정보가 옴
					{
						// 받은 메시지 유저한테 전송
						EnterCriticalSection(&UserListSync);
						for (auto User : UserList)
						{
							send(User, Buffer, recvlen, 0);
						}
						LeaveCriticalSection(&UserListSync);
					}
				}

			}
		}
	}
	DeleteCriticalSection(&UserListSync);
}
