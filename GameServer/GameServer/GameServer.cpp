//==============================================
// GAMESERVERCODE
// =============================================
// 목차: Completion Port 모델
//==============================================
#include "pch.h"
#include "CorePch.h"
#include "ThreadManager.h"
#include "CoreMacro.h"

	//overlapped 모델 (completion Routine 콜백 기반)
	// - 비동기 입출력 함수 완료 되면 , 쓰레드 마다 있는 APC 큐에 일감이 쌓임
	// - Alertable Wait 상태로 들어가서 APC 큐 비우기 (콜백 함수)
	// 단점) APC  큐 쓰레드 마다 있따.! ALERTABLE Wait 자체도 조금 부담
	// 단점) 이벤트 방식 소켓: 이벤트 1:! 대응

	//IOCP (Completion Port) 모델
	// -APC-> Completion Port (쓰레드마다 있는건 아니고 1개, 중앙에서 관리하는 APC큐?)
	// - Alertable Wait -> CP 결과처리를 GetQueueCompletionStatus 
	// - 쓰레드랑 궁합이 굉장히 좋다.

	//CreateIOCompletionPort
	//GetQueueCompletionStatus


//마이크로소프트에서 제공하는 소켓 라이브러리
#include <WinSock2.h>
#include <MSWSock.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")




CoreGlobal Core;

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
enum IO_TYPE {
	READ,
	WRITE,
	ACCEPT,
	CONNECT,
};

struct OverlappedEx {

	WSAOVERLAPPED overlapped = {};
	int32 type = 0; //read, write, accept ,connet...
};

void CALLBACK RecvCallback(DWORD error, DWORD recvLen, LPWSAOVERLAPPED overlapped, DWORD flags) {

	cout << "Data Recv Len Callback" << recvLen << endl;
	//TODO : 에코 서버를 만든다면 WSASend()

	Session* session = (Session*)overlapped;
}

void WorkerThreadMain(HANDLE iocpHandle) {

	while (true) {

		DWORD bytesTransferred = 0;
		Session* session = nullptr;
		OverlappedEx* overlappedEx = nullptr;

		//루프를 돌면서 일감이 있는지 찾고있다!
		bool ret = ::GetQueuedCompletionStatus(iocpHandle, &bytesTransferred,
			(ULONG_PTR*)&session, (LPOVERLAPPED*)&overlappedEx, INFINITE);

		if (ret == FALSE || bytesTransferred == 0) {
			continue;
		}


		ASSERT_CRASH(overlappedEx->type == IO_TYPE::READ);


		cout << "Recv Data IOCP" << bytesTransferred << endl;


		WSABUF wsaBuf;
		wsaBuf.buf = session->recvBuffer;
		wsaBuf.len = BUFSIZE;


		DWORD recvLen = 0;
		DWORD flags = 0;
		::WSARecv(session->socket, &wsaBuf, 1, &recvLen, &flags, &overlappedEx->overlapped, NULL);

	}
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


	vector<Session*> sessionManager;


	//1.CP 생성 (중앙 컨트롤 같은 아이)
	HANDLE iocpHandle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

	//WorkerThreads
	for (int32 i = 0; i < 5; i++) {
		GThreadManager->Launch([=]() {WorkerThreadMain(iocpHandle); });
	}
	cout << "makeThread" << endl;
	//Main Thread =Accept 담당
	while (true) {
		
		SOCKADDR_IN clientAddr;
		int32 addrLen = sizeof(clientAddr);

		 
		SOCKET clientSocket = ::accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen);
		//cout << "makeSoket" <<clientSocket<< endl;
		if (clientSocket == INVALID_SOCKET)
			return 0;
		//cout << "socketerrorCheck" << endl;
		Session* session = new Session();
		session->socket = clientSocket;
		sessionManager.push_back(session);


		cout << "Client Conneted ! " << endl;

		//소켓을 CP에 등록
		CreateIoCompletionPort((HANDLE)clientSocket, iocpHandle,/*keyvalue*/(ULONG_PTR)session, 0);

		WSABUF wsaBuf;
		wsaBuf.buf = session->recvBuffer;
		wsaBuf.len = BUFSIZE;
		OverlappedEx* overlappedEx = new OverlappedEx();
		overlappedEx->type = IO_TYPE::READ;

	
		DWORD recvLen = 0;
		DWORD flags = 0;
		::WSARecv(clientSocket, &wsaBuf, 1, &recvLen, &flags,&overlappedEx->overlapped, NULL);
		
		//유저가 게임 종료!
		//Session* s = sessionManager.back();
		//sessionManager.pop_back();
		//delete s;
		
		//해결법
		//1.ref카운팅을 해준다.
		//키값(session)같은 값이 예약을 해둿다면 날라가지않게 보존 시켜줘야한다.!

		//closesocket(clientSocket);
		//WSACloseEvent(wsaEvent);
	}

	GThreadManager->Join();


	//윈속 종료.
	::WSACleanup();
}

