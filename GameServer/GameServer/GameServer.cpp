//==============================================
// GAMESERVERCODE
// =============================================
// 목차: WSAEventSelect
//==============================================
// Select는 매번마다 초기화를 했어야하는 코드를 했어야했지만, 이 모델은 이벤트를 통해서 해결함
// Select와 마찬가지로 이벤트를 최대 64개를 만들수 있어서 이건 염두해두고 해야함.

	//WASEventSelect = (WSAEventselect 함수가 핵심이 되는)
	//소켓과 관련된 네트워크 이벤트를 [이벤트 객체]를 통해 감지

	//이벤트 객체 관련 함수들
	// 생성 : WSACreateEvent (수동 리셋 Manual -Reset + Non -Signaled 상태 시작)
	// 삭제 : WSACloseEvent
	//신호 상태 감지: WSAWaitForMultipleEvents
	//구체적인 네트워크 이벤트 알아내기 : WSAEnumNetworkEvents

	//소켓 <=> 이벤트 객체 연동
	//WSAEventSelect (socket ,event , networkEvents)
	// - 관심 잇는 네트워크 이벤트
	// FD_ACCEPT : 접속한 클라가 있음 accept
	//FD_READ	:데이터 수신 가능 recv , recvfrom
	//FD_WRITE	:데이터 송신 가능 send , sendto
	// FD_CLOSE :상대가 접속 종료
	//FD_CONNET	:통신을 위한 연결 절차 완료
	//FD_OOB

	//주의 사항
	//WSAEventSelect 함수를 호출하면, 해당 소켓은 자동으로 넌블로킹 모드 전환
	// accept() 함수가 리턴하는 소켓은 listenSocket과 동일한 속성을 갖는다.
	//-따라서 clientSocket은 FD_READ , FD_WRITE 등을 다시 등록 필요
	// 드물게 WSAEWOULDBLOCK 오류가 뜰 수 있으니 예외처리 필요
	// 중요)
	// -이벤트 발생시, 적절한 소켓 함수 호출해야함.
	//- 아니면 다음번에 동일 네트워크 이벤트가 발생 X
	// ex) FD_READ 이벤트 떳으면 recv() 호출해야하고, 안하면 FD_READ 두번 다시 X

	//1 ) count , event
	//2 ) waitAll: 모두 기다림? 하나만 완료 되어도 OK?
	//3 ) time out : 타임 아웃
	//4 ) 지금은 false
	// return :완료된 첫번째 인덱스

	//1) socket
	//2) eventObject : socket과 연동된 이벤트 객체 핸들을 넘겨주면, 이벤트 객체를 non - signealed
	//3) networkEvent : 네트워크 이벤트  /오류 정보가 저장


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
const int32 BUFSIZE = 1000;

//동접수만큼 만들어짐 session은
struct Session {

	SOCKET socket;
	char recvBuffer[BUFSIZE] = {};
	int32 recvBytes = 0;
	int32 sendBytes = 0;

};


int main()
{
	save("GameServer.cpp", "ServerCodeFile.txt");
	cout << "Server" << endl;


	WSADATA wsData;
	if (::WSAStartup(MAKEWORD(2, 2), &wsData))
		return 0;

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

	cout << "Accept" << endl;


	//WSAWaitForMultipleEvents

	vector<WSAEVENT> wsaEvents;
	vector<Session> sessions;
	sessions.reserve(100);

	WSAEVENT listenEvent = ::WSACreateEvent();
	wsaEvents.push_back(listenEvent);
	sessions.push_back(Session{ listenSocket });

	if (::WSAEventSelect(listenSocket, listenEvent, FD_ACCEPT | FD_CLOSE) == SOCKET_ERROR)
		return 0;


	while (true) {
		//인덱스가 반환됨
		int32 index = ::WSAWaitForMultipleEvents(wsaEvents.size(), &wsaEvents[0], FALSE, WSA_INFINITE, FALSE);
		if (index == WSA_WAIT_FAILED)
			continue;


		index -= WSA_WAIT_EVENT_0;

		//::WSAResetEvent() 이것을 넣어도 되고 안되도 되지만 , WSAEnumNetworkEvents에 들어가 잇음

		WSANETWORKEVENTS networkEvents;
		if (::WSAEnumNetworkEvents(sessions[index].socket, wsaEvents[index], &networkEvents) == SOCKET_ERROR)
			continue;


		//listener 소켓 체크
		if (networkEvents.lNetworkEvents & FD_ACCEPT) {

			//Error - check
			if (networkEvents.iErrorCode[FD_ACCEPT_BIT] != 0)
				continue;

			SOCKADDR_IN clientAddr;
			int32 addrLen = sizeof(clientAddr);

			SOCKET clientSocket = ::accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen);
			if (clientSocket != INVALID_SOCKET) {

				cout << "Client Conneted" << endl;

				WSAEVENT clientEvent = ::WSACreateEvent();
				wsaEvents.push_back(clientEvent);
				sessions.push_back(Session{ clientSocket });
				//클라언트 소켓이랑 리슨 이벤트랑 연동시켜줌
				if (::WSAEventSelect(clientSocket, clientEvent, FD_READ | FD_WRITE | FD_CLOSE) == SOCKET_ERROR)
					return 0;
			}
		}

		//Client Session 소켓 체크
		if (networkEvents.lNetworkEvents & FD_READ || networkEvents.lNetworkEvents & FD_WRITE) {

			//Error - Check
			if ((networkEvents.lNetworkEvents & FD_READ) && (networkEvents.iErrorCode[FD_READ_BIT] != 0))
				continue;
			//Error - Check
			if ((networkEvents.lNetworkEvents & FD_WRITE) && (networkEvents.iErrorCode[FD_WRITE_BIT] != 0))
				continue;

			Session& s = sessions[index];

			if (s.recvBytes == 0) {
				int32 recvLen = ::recv(s.socket, s.recvBuffer, BUFSIZE, 0);
				if (recvLen == SOCKET_ERROR && ::WSAGetLastError() != WSAEWOULDBLOCK) {


					//TODO :Remove Session
					continue;

				}

				s.recvBytes = recvLen;
				cout << "Recv Data=" << recvLen << endl;

			}


			//Write
			if (s.recvBytes > s.sendBytes) {

				int32 sendLen = ::send(s.socket, &s.recvBuffer[s.sendBytes], s.recvBytes - s.sendBytes, 0);

				if (sendLen == SOCKET_ERROR && ::WSAGetLastError() != WSAEWOULDBLOCK) {

					cout << "sendLen error" << endl;
					continue;
				}

				s.sendBytes += sendLen;
				if (s.recvBytes == s.sendBytes) {

					s.recvBytes = 0;
					s.sendBytes = 0;
				}

				cout << "Send Data =" << sendLen << endl;

			}
		}

		//FD_CLOSE 처리
		if (networkEvents.lNetworkEvents & FD_CLOSE) {

			//TODO: remove Socket
		}
	}



	//윈속 종료.
	::WSACleanup();
}

