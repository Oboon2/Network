#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib")
#define PORT 4000
// ������ ���� �ڵ� ���
// ������ �̺�Ʈ ���, read, send, accept
//while (true)
//{
	// select()
	// �����ϴ� ���Ͽ� �̺�Ʈ�� �߻��ߴ��� Ȯ��, ��� ���缭 TIMEVAL �ð�
	// �̺�Ʈ�� �߻��ϸ� ����ü�� �̺�Ʈ�� �߻��� �����̶�, �̺�Ʈ
	// �̺�Ʈ ó��
	// ���� �ϴ��� ó��
	// if a = select(), 2000��
	// if a.read
	// {
	//
	// }
	// else if a.send
	// {
	//
	// }
	// ���� ó��
	// ���� ������ �۾�
	// OS��� ������Ʈ()
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

	// WinSock �ʱ�ȭ
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
	ServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);  // �ƹ� �����ǳ�

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
			// �������� �ϴ� �ٸ� �۾�
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
					// Ŭ���̾�Ʈ�� ��ٸ�
					hClientSocket = accept(hServerSocket, (SOCKADDR*)&ClientAddress, &ClientSocketAddrSize); // ��ٷ�,  ���ŷ blocking socket
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
					int RecvCount = recv(Reads.fd_array[i], Buffer, sizeof(Buffer) - 1, 0); // ���ŷ
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
							RecvCount = recv(hClientSocket, Buffer, sizeof(Buffer) - 1, 0); // ���ŷ
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