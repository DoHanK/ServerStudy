//==============================================
// GAMESERVERCODE
// =============================================
// 목차: Overlapped 모델 (콜백 기반)
//==============================================



	//Overlapped 모델 (Completion Routine 콜백 기반)
	// - 비동기 입출력 지원하는 소켓 생성 
	// - 비동기 입출력 함수 호출 (완료 루틴의 시작 주소를 넘겨준다.)
	// - 비동기 작업이 바로 완료되지 않으면 , WSA_IO_PENDING 오류 코드
	// - 비동기 입출력 함수 호출한 쓰레드를 -Alertable Wait 상태로 만든다.
	// ex) WaitForSingleObjectEx , WaitForMultipleObjectsEx , SleepEx, WSAWAitForMultipleEvents
	//  비동기 IO 완료되면 ,운영체제는 완료 루틴 호출
	// -완료 루틴 호출이 모두 끝나면, 쓰레드는 alertavle Wait 상태에서 빠져나온다.





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
	WSAOVERLAPPED overlapped = {  };
};

void CALLBACK RecvCallback(DWORD error, DWORD recvLen, LPWSAOVERLAPPED overlapped, DWORD flags) {

	cout << "Data Recv Len Callback" << recvLen << endl;
	//TODO : 에코 서버를 만든다면 WSASend()

	Session* session = (Session*)overlapped;
}

int main()
{
	//save("GameServer.cpp", "ServerCodeFile.txt");
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
	// 1) 오류 발생시 0이 아닌값
	// 2) 전송 바이트 수
	// 3) 비동기 입출력 함수 호출시 넘겨준 WSAOVERLAPPED 구조체의 주소값
	// 4) 0
	//void  CompletionRoutine()

	//select 모델
	// -장점 ) 윈도우/ 리눅스 공통으로 사용 가능!
	// -단점 ) 성능 최하 (매번 등록 비용) , 소켓 64개 제한 
	//WSAAsynSelect 모델 = 소켓 이벤트를 윈도우 메세지 형태로 처리 (일반 윈도우 메시지랑 같이 처리하니 성능~~_
	// WSAEventSelect 모델 
	// -장점 ) 비교적 뛰어난 성능
	// -단점 ) 64개 제한
	// Overlapped(이벤트 기반)
	// - 장점 ) 성능 
	// - 단점 ) 64개 제한
	// Overlapped(콜백 기반)
	// - 장점) 성능
	// - 단점) 모든 비동기 소켓 함수에서 사용 가능하진 않음 (accept) .빈벉한 alertable Wait 으로 인한 성능 저하

	//끝판왕 IOCP

	//Reactor Pattern (~뒤늦게. 논 블로킹 소켓 . 소켓 상태확인후 -> 뒤늦게 recv send 호출)
	// Protactor Pattern (~미리. Overlapped WSA~)
	while (true) {
		
		SOCKADDR_IN clientAddr;
		int32 addrLen = sizeof(clientAddr);

		 
		SOCKET clientSocket;
		while (true) {
			clientSocket = ::accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen);
			if (clientSocket != INVALID_SOCKET)
				break;

			
			if (::WSAGetLastError() == WSAEWOULDBLOCK)
				continue;

			//문제 있는 상황
			return 0;
		}

		Session session = Session{ clientSocket };
		WSAEVENT wsaEvent = ::WSACreateEvent();
		session.overlapped.hEvent = wsaEvent;

		cout << "Client Conneted ! " << endl;

		while (true) {

			WSABUF wsaBuf;
			wsaBuf.buf = session.recvBuffer;
			wsaBuf.len = BUFSIZE;
			
			DWORD recvLen = 0;
			DWORD flags = 0;
			if (::WSARecv(clientSocket, &wsaBuf, 1, &recvLen, &flags, &session.overlapped,RecvCallback) == SOCKET_ERROR) {
				
				if (::WSAGetLastError() == WSA_IO_PENDING) {

					//pending
					//Alertable Wait

					//::WSAWaitForMultipleEvents(1, &wsaEvent, TRUE, WSA_INFINITE, TRUE);
					::SleepEx(INFINITE, TRUE); // <- Alertable Wait
				}
				else {

					//TODO: 문제있는 상황
					break;
				}
			}
			else {

			cout << "Data Recv Len =" << recvLen << endl;
			
			}



		}
		closesocket(clientSocket);
		WSACloseEvent(wsaEvent);
	}



	//윈속 종료.
	::WSACleanup();
}

