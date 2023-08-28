//==============================================
// CLIENT CODE
//==============================================
// 목차: Completion Port 모델
//==============================================


#include "pch.h"
#include <iostream>

//마이크로소프트에서 제공하는 소켓 라이브러리
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
	//save("DummyClient.cpp", "ClientCodeFile.txt");
	cout << "Client" << endl;


	WSADATA wsData;
	if (::WSAStartup(MAKEWORD(2, 2), &wsData))
		return 0;


	SOCKET clientSocket = ::socket(AF_INET, SOCK_STREAM, 0);
	if (clientSocket == INVALID_SOCKET)
		return 0;


	u_long on = 1;
	if (::ioctlsocket(clientSocket, FIONBIO, &on) == INVALID_SOCKET)
		return 0;


	SOCKADDR_IN  serverAddr;
	::memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	::inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
	serverAddr.sin_port = ::htons(7777);


	//Connet
	while (true) {

		if (::connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
		{
			if (::WSAGetLastError() == WSAEWOULDBLOCK)
				continue;
			
			if (::WSAGetLastError() == WSAEISCONN)
				break;

			//Error
			break;
		}
	}
	

	cout << "conneted to server!" << endl;
	char sendBuffer[100] = "hello World";
	WSAEVENT wsaEvent = ::WSACreateEvent();
	WSAOVERLAPPED overlapped = {};
	overlapped.hEvent = wsaEvent;

	//send
	while (true) {

		WSABUF wsaBuf;
		wsaBuf.buf = sendBuffer;
		wsaBuf.len = 100;

		DWORD sendLen = 0;
		DWORD flags = 0;	
		if (::WSASend(clientSocket, &wsaBuf, 1, &sendLen, flags, &overlapped, nullptr) == SOCKET_ERROR) {
			
			if (::WSAGetLastError() == WSA_IO_PENDING) {

				//Pending
				::WSAWaitForMultipleEvents(1, &wsaEvent, TRUE, WSA_INFINITE, FALSE);
				::WSAGetOverlappedResult(clientSocket, &overlapped, &sendLen, FALSE, &flags);
			}
			else {
				//진짜 문제 있는 상황
				break;
			}

		}


		cout << "send Data! Len" << sizeof(sendBuffer) << endl;

		

		
		Sleep(1000);
	}

	//소켓 리소스 반환
	closesocket(clientSocket);


	//윈속 종료.
	::WSACleanup();




}

