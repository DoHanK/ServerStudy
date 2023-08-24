//==============================================
// CLIENT CODE
//==============================================
// 목차: 논블로킹 소켓
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

	SOCKET clientSocket = ::socket(AF_INET, SOCK_DGRAM, 0);
	// 소문자로 리눅스에 있을수도 있고, 대문자는 윈속에서만 대부분 제공하는것
	if (clientSocket == INVALID_SOCKET) {
		HandleError("Socket");
		return 0;
	}

	

	//소켓 리소스 반환
	closesocket(clientSocket);


	//윈속 종료.
	::WSACleanup();




}

