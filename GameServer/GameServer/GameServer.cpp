//==============================================
// GAMESERVERCODE
// =============================================
// ����: Completion Port ��
//==============================================
#include "pch.h"
#include "CorePch.h"
#include "ThreadManager.h"
#include "CoreMacro.h"

	//overlapped �� (completion Routine �ݹ� ���)
	// - �񵿱� ����� �Լ� �Ϸ� �Ǹ� , ������ ���� �ִ� APC ť�� �ϰ��� ����
	// - Alertable Wait ���·� ���� APC ť ���� (�ݹ� �Լ�)
	// ����) APC  ť ������ ���� �ֵ�.! ALERTABLE Wait ��ü�� ���� �δ�
	// ����) �̺�Ʈ ��� ����: �̺�Ʈ 1:! ����

	//IOCP (Completion Port) ��
	// -APC-> Completion Port (�����帶�� �ִ°� �ƴϰ� 1��, �߾ӿ��� �����ϴ� APCť?)
	// - Alertable Wait -> CP ���ó���� GetQueueCompletionStatus 
	// - ������� ������ ������ ����.

	//CreateIOCompletionPort
	//GetQueueCompletionStatus


//����ũ�μ���Ʈ���� �����ϴ� ���� ���̺귯��
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

//��������ŭ ������� session��
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
	//TODO : ���� ������ ����ٸ� WSASend()

	Session* session = (Session*)overlapped;
}

void WorkerThreadMain(HANDLE iocpHandle) {

	while (true) {

		DWORD bytesTransferred = 0;
		Session* session = nullptr;
		OverlappedEx* overlappedEx = nullptr;

		//������ ���鼭 �ϰ��� �ִ��� ã���ִ�!
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


	//1.CP ���� (�߾� ��Ʈ�� ���� ����)
	HANDLE iocpHandle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

	//WorkerThreads
	for (int32 i = 0; i < 5; i++) {
		GThreadManager->Launch([=]() {WorkerThreadMain(iocpHandle); });
	}
	cout << "makeThread" << endl;
	//Main Thread =Accept ���
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

		//������ CP�� ���
		CreateIoCompletionPort((HANDLE)clientSocket, iocpHandle,/*keyvalue*/(ULONG_PTR)session, 0);

		WSABUF wsaBuf;
		wsaBuf.buf = session->recvBuffer;
		wsaBuf.len = BUFSIZE;
		OverlappedEx* overlappedEx = new OverlappedEx();
		overlappedEx->type = IO_TYPE::READ;

	
		DWORD recvLen = 0;
		DWORD flags = 0;
		::WSARecv(clientSocket, &wsaBuf, 1, &recvLen, &flags,&overlappedEx->overlapped, NULL);
		
		//������ ���� ����!
		//Session* s = sessionManager.back();
		//sessionManager.pop_back();
		//delete s;
		
		//�ذ��
		//1.refī������ ���ش�.
		//Ű��(session)���� ���� ������ �؊x�ٸ� �������ʰ� ���� ��������Ѵ�.!

		//closesocket(clientSocket);
		//WSACloseEvent(wsaEvent);
	}

	GThreadManager->Join();


	//���� ����.
	::WSACleanup();
}

