//==============================================
// CLIENT CODE
//==============================================
// 목차: UDP 코드 작성
//==============================================


#include "pch.h"
#include <iostream>

//마이크로소프트에서 제공하는 소켓 라이브러리
#include <WinSock2.h>
#include <MSWSock.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")



int main()
{
	//save("DummyClient.cpp", "ClientCodeFile.txt");

	//윈속 초기화 (WS2_32 라이브러리 초기화)
	//관련 정보가 wsaData에 채워짐

	WSADATA wsData;
	if (::WSAStartup(MAKEWORD(2, 2), &wsData))
		return 0;

	//af(Address Family)->  (AF_INET) IPv4 , (AF_INET6) IPv6
	//tpye -> tcp냐 , udp냐를 선택하는것
	//protocol 선택 0을 주면 알아서 정해줌
	//return :descriptor 포인터를 리턴해준다 정도로 알면됨
	SOCKET clientSocket = ::socket(AF_INET, SOCK_STREAM, 0);
	// 소문자로 리눅스에 있을수도 있고, 대문자는 윈속에서만 대부분 제공하는것
	if (clientSocket == INVALID_SOCKET) {

		int32 errCode = ::WSAGetLastError();
		cout << "SocketErrorCode" << endl;
		return 0;
	}

	SOCKADDR_IN	serverAddr;//IPv4
	::memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	//serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); //derprecated 하다
	::inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
	serverAddr.sin_port = htons(7777);
	

	////////////////////////////////////////////////////////////////////
	//host to network short 
	//숫자를 저장할때 두가지 방법
	//LIttle-Endian VS Big-Endian
	// ex) 0x12345678
	//low  78 56 34 12   high ->리틀 엔디안	 - >인텔은 리틀 엔디안을 사용함.
	//low 12 34 56 78    high -> 빅 엔디안   ->네트워크에서 채택한 방식
	/////////////////////////////////////////////////////////////////////


	if(::connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)))
	{
		int32 errCode = ::WSAGetLastError();
		cout << "SocketErrorCode" << endl;
		return 0;
	}



		//==============================================
		// 연결 성공! 이제부터 데이터 송수신이 가능하다.!
		//===============================================
	while (true) {

		//==============================================
		// 통신 코드 작성
		//===============================================


		//==============================================
		//클라 - > 서버 데이터 전송 코드
		//===============================================
		char sendBuffer[100] = "hello World";
		for (int32 i = 0; i < 10; i++) {
			int32 resultCode = ::send(clientSocket, sendBuffer, sizeof(sendBuffer), 0);
			if (resultCode == SOCKET_ERROR) {
				int32 errCode = ::WSAGetLastError();
				cout << "send ErrorCode:  " << errCode << endl;
				return 0;
			}
		}

		cout << "sendData! len= " << sizeof(sendBuffer) << endl;
		////////////////////////////////////////////////////////////


		char recvBuffer[1000];
		//==============================================
		// 서버 - >클라 데이터 리시브 코드
		//===============================================

 
		//받은 데이터의 크기를 반환해줌 recv 데이터는 -1을 반환하면.문제가 생김
		int32 recvLen = ::recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);
		if (recvLen <= 0) {
			int32 errCode = ::WSAGetLastError();
			cout << "Recv ErrorCode" << endl;
			return 0;

		}

		cout << "Recv Data! Data == " << recvBuffer << endl;
		cout << "Recv Data! len == " << recvLen << endl;
		////////////////////////////////////////////////////////////
		
	}


	//소켓 리소스 반환
	closesocket(clientSocket);


	//윈속 종료.
	::WSACleanup();




}

