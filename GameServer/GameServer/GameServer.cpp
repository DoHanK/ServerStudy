﻿// GameServer.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//
#include "pch.h"
#include <iostream>


//마이크로소프트에서 제공하는 소켓 라이브러리
#include <WinSock2.h>
#include <MSWSock.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")


int main()
{
	WSADATA wsData;
	if (::WSAStartup(MAKEWORD(2, 2), &wsData))
		return 0;


	SOCKET listenSocket = ::socket(AF_INET, SOCK_STREAM, 0);
	if (listenSocket == INVALID_SOCKET) {

		int32 errCode = ::WSAGetLastError();
		cout << "SocketErrorCode" << endl;
		return 0;
	}

	//회사의 주소를 찍어줘야함(나의 주소는?)
	SOCKADDR_IN	serverAddr;//IPv4
	::memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(7777);

	//안내원의 폰을 개통해줘야함.!
	if (::bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		int32 errCode = ::WSAGetLastError();
		cout << "SocketErrorCode" << endl;
		return 0;
	}


	//영업 시작
	if (::listen(listenSocket, 10)) {
		int32 errCode = WSAGetLastError();
		cout << "Listen ErrorCode" << endl;
		return 0;
	}

	//-------------------------

	while (true) {
		SOCKADDR_IN	clientAddr;//IPv4
		::memset(&clientAddr, 0, sizeof(serverAddr));
		int32 addrLen = sizeof(clientAddr);
		
		
		
		SOCKET clientSocket = ::accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen);
		//cleint소켓이 데이터를 주고 받을때 사용함!!!
		if (clientSocket == INVALID_SOCKET) {
			int32 errCode = ::WSAGetLastError();
			cout << "Accept Error" << errCode << endl;
			return 0;	SOCKADDR_IN	serverAddr;//IPv4
			::memset(&serverAddr, 0, sizeof(serverAddr));
		}


		char ipAddress[16];
		::inet_ntop(AF_INET, &clientAddr.sin_addr, ipAddress, sizeof(ipAddress));
		cout << "client Conneted! IP" << ipAddress << endl;



	}

	//-------------------------

	//소켓 리소스 반환
	closesocket(listenSocket);

	//윈속 종료.
	::WSACleanup();
}

