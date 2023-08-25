//==============================================
// GAMESERVERCODE
// =============================================
// ����: select ��
//==============================================
#include "pch.h"
#include <iostream>


//����ũ�μ���Ʈ���� �����ϴ� ���� ���̺귯��
#include <WinSock2.h>
#include <MSWSock.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

void HandleError(const char* cause) {
	int32 errCode = ::WSAGetLastError();
	cout << cause << "ErrorCode" << errCode << endl;

}

int main()
{
	//save("GameServer.cpp", "ServerCodeFile.txt");
	cout << "Server" << endl;


	WSADATA wsData;
	if (::WSAStartup(MAKEWORD(2, 2), &wsData))
		return 0;

	//���ŷ(Blocking) ����
	// accept -> ������ Ŭ�� ������
	//connet-> ���� ���� ����������
	//send , sendto -> ��û�� �����͸� �۽� ���ۿ� ����������
	//recv, recvfrom -> ���� ���ۿ� ������ �����Ͱ� �ְ�, �̸� ���� ���� ���ۿ� ����������
	
	//�� ���ŷ(Non- Blocking)

	SOCKET listenSocket = ::socket(AF_INET, SOCK_STREAM, 0);
	if (listenSocket == INVALID_SOCKET)
	{
		return 0;
	}


	u_long on = 1;
	if (::ioctlsocket(listenSocket, FIONBIO, &on) == INVALID_SOCKET)
		return 0;
	
	
	SOCKADDR_IN serverAddr;
	::memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(7777);


	if (::bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
		return 0;


	if (::listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
		return 0;


		SOCKADDR_IN clientAddr;
		int32 addrLen = sizeof(clientAddr);


		//Accept
		while (true) {

			SOCKET clientSocket = ::accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen);
			if (clientSocket == INVALID_SOCKET) {

				//���� ��� �߾���ߴµ�...�ʰ� ����ŷ���� �϶��
				if (::WSAGetLastError() == WSAEWOULDBLOCK)
					continue;

				//Error
				break;
			}

			cout << "client Conneted!" << endl; 

			//recv
			while (true) {

				char recvBuffer[1000];
				int32 recvLen = ::recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);

				if (recvLen == SOCKET_ERROR) {

					if (::WSAGetLastError() == WSAEWOULDBLOCK)
						continue;

					break;
				}
				else if (recvLen == 0) {
					//���� ������
					break;
				}

				cout << "Recv Data Len = " << recvLen << endl;

				//send
				while (true) {

					if (::send(clientSocket, recvBuffer, recvLen, 0) == SOCKET_ERROR) {


						//���� ��� �߾���ߴµ�.. �ʰ� �� ���ŷ���� �϶��,...
						if (::WSAGetLastError() == WSAEWOULDBLOCK)
							continue;

						//Error
						break;
					}

					cout << "send Data! Len = " << recvLen << endl;
				}
			}
			
		}

	//���� ����.
	::WSACleanup();
}

