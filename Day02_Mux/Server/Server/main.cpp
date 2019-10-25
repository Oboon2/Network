#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib")
#define PORT 4000
// 감시할 소켓 핸들 등록
// 감시할 이벤트 등록, read, send, accept
//while (true)
//{
	// select()
	// 감시하는 소켓에 이벤트가 발생했는지 확인, 잠시 멈춰서 TIMEVAL 시간
	// 이벤트가 발생하면 구조체에 이벤트가 발생한 소켓이랑, 이벤트
	// 이벤트 처리
	// 서버 하는일 처리
	// if a = select(), 2000개
	// if a.read
	// {
	//
	// }
	// else if a.send
	// {
	//
	// }
	// 소켓 처리
	// 서버 나머지 작업
	// OS기능 업데이트()
//}
int main()
{
	WSADATA wsaData;
	SOCKET hServerSocket;
	SOCKET hClientSocket;
	struct sockaddr_in ServerAddress;
	SOCKADDR ClientAddress;

	TIMEVAL TimeOut;
	fd_set Reads;
	// fd_set Writes;
	// fd_set Exceptions;
	fd_set CopyReads;

	// WinSock 초기화
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("Winsocket Error\n");
		exit(-1);
	}

	hServerSocket = socket(PF_INET, SOCK_STREAM, 0);
	if (hServerSocket == INVALID_SOCKET)
	{
		printf("socket() error %d\n",  GetLastError());
		exit(-1);
	}

	memset(&ServerAddress, 0, sizeof(SOCKADDR));
	ServerAddress.sin_family = PF_INET;
	ServerAddress.sin_port = htons(PORT);
	ServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);  // 아무 아이피나

	if (bind(hServerSocket, (SOCKADDR*)&ServerAddress, sizeof(ServerAddress)) == SOCKET_ERROR)
	{
		printf("bind() error %d\n", GetLastError());
		exit(-1);
	}
	if (listen(hServerSocket, 5) == SOCKET_ERROR)
	{
		printf("listen() error %d\n", GetLastError());
		exit(-1);
	}

	FD_ZERO(&Reads);
	FD_SET(hServerSocket, &Reads);


	char Buffer[1024*8];

	int ClientSocketAddrSize = sizeof(ClientAddress);
	while (true)
	{
		CopyReads = Reads;
		TimeOut.tv_sec = 5;
		TimeOut.tv_usec = 5000;

		int fdReadCount = select(0, &CopyReads, 0, 0, &TimeOut);

		if (fdReadCount == SOCKET_ERROR)
		{
			printf("select error\n");
			exit(-1);
		}

		if (fdReadCount == 0)
		{
			// 서버에서 하는 다른 작업
			// printf("TimeOut\n");
			continue;
		}
		
		for (int i = 0; i < (int)Reads.fd_count; ++i)
		{
			if (FD_ISSET(Reads.fd_array[i], &CopyReads))
			{
				// connect client
				if (Reads.fd_array[i] == hServerSocket)
				{
					// 클라이언트가 기다림
					hClientSocket = accept(hServerSocket, (SOCKADDR*)&ClientAddress, &ClientSocketAddrSize); // 기다려,  블락킹 blocking socket
					if (hClientSocket == INVALID_SOCKET)
					{
						printf("accept() error %d\n", GetLastError());
						exit(-1);
					}
					FD_SET(hClientSocket, &Reads);
				}
				else
				{
					// client send, process
					memset(Buffer, 0, sizeof(Buffer));
					int RecvCount = recv(Reads.fd_array[i], Buffer, sizeof(Buffer) - 1, 0); // 블락킹
					if (RecvCount <= 0)
					{
						printf("Client Disconnect\n");
						FD_CLR(Reads.fd_array[i], &Reads);
						closesocket(Reads.fd_array[i]);
						break;
					}
					else
					{
						if (strcmp(Buffer, "end") == 0 || strcmp(Buffer, "End") == 0 || strcmp(Buffer, "End") == 0)
						{
							shutdown(hClientSocket, SD_SEND);
							RecvCount = recv(hClientSocket, Buffer, sizeof(Buffer) - 1, 0); // 블락킹
							FD_CLR(Reads.fd_array[i], &Reads);
							closesocket(Reads.fd_array[i]);
							
						}
						else
						{
							for (int j = 1; j < (int)Reads.fd_count; ++j)
							{
								printf("Client Send %s\n", Buffer);
								send(Reads.fd_array[j], Buffer, RecvCount, 0);
							}
							
						}
						
					}
				}
			}
		}
	}

	closesocket(hServerSocket);
	WSACleanup();

	return 0;
}