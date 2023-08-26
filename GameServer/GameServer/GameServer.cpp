//==============================================
// GAMESERVERCODE
// =============================================
// ����: WSAEventSelect
//==============================================
// Select�� �Ź����� �ʱ�ȭ�� �߾���ϴ� �ڵ带 �߾��������, �� ���� �̺�Ʈ�� ���ؼ� �ذ���
// Select�� ���������� �̺�Ʈ�� �ִ� 64���� ����� �־ �̰� �����صΰ� �ؾ���.

	//WASEventSelect = (WSAEventselect �Լ��� �ٽ��� �Ǵ�)
	//���ϰ� ���õ� ��Ʈ��ũ �̺�Ʈ�� [�̺�Ʈ ��ü]�� ���� ����

	//�̺�Ʈ ��ü ���� �Լ���
	// ���� : WSACreateEvent (���� ���� Manual -Reset + Non -Signaled ���� ����)
	// ���� : WSACloseEvent
	//��ȣ ���� ����: WSAWaitForMultipleEvents
	//��ü���� ��Ʈ��ũ �̺�Ʈ �˾Ƴ��� : WSAEnumNetworkEvents

	//���� <=> �̺�Ʈ ��ü ����
	//WSAEventSelect (socket ,event , networkEvents)
	// - ���� �մ� ��Ʈ��ũ �̺�Ʈ
	// FD_ACCEPT : ������ Ŭ�� ���� accept
	//FD_READ	:������ ���� ���� recv , recvfrom
	//FD_WRITE	:������ �۽� ���� send , sendto
	// FD_CLOSE :��밡 ���� ����
	//FD_CONNET	:����� ���� ���� ���� �Ϸ�
	//FD_OOB

	//���� ����
	//WSAEventSelect �Լ��� ȣ���ϸ�, �ش� ������ �ڵ����� �ͺ��ŷ ��� ��ȯ
	// accept() �Լ��� �����ϴ� ������ listenSocket�� ������ �Ӽ��� ���´�.
	//-���� clientSocket�� FD_READ , FD_WRITE ���� �ٽ� ��� �ʿ�
	// �幰�� WSAEWOULDBLOCK ������ �� �� ������ ����ó�� �ʿ�
	// �߿�)
	// -�̺�Ʈ �߻���, ������ ���� �Լ� ȣ���ؾ���.
	//- �ƴϸ� �������� ���� ��Ʈ��ũ �̺�Ʈ�� �߻� X
	// ex) FD_READ �̺�Ʈ ������ recv() ȣ���ؾ��ϰ�, ���ϸ� FD_READ �ι� �ٽ� X

	//1 ) count , event
	//2 ) waitAll: ��� ��ٸ�? �ϳ��� �Ϸ� �Ǿ OK?
	//3 ) time out : Ÿ�� �ƿ�
	//4 ) ������ false
	// return :�Ϸ�� ù��° �ε���

	//1) socket
	//2) eventObject : socket�� ������ �̺�Ʈ ��ü �ڵ��� �Ѱ��ָ�, �̺�Ʈ ��ü�� non - signealed
	//3) networkEvent : ��Ʈ��ũ �̺�Ʈ  /���� ������ ����


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
const int32 BUFSIZE = 1000;

//��������ŭ ������� session��
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
		//�ε����� ��ȯ��
		int32 index = ::WSAWaitForMultipleEvents(wsaEvents.size(), &wsaEvents[0], FALSE, WSA_INFINITE, FALSE);
		if (index == WSA_WAIT_FAILED)
			continue;


		index -= WSA_WAIT_EVENT_0;

		//::WSAResetEvent() �̰��� �־ �ǰ� �ȵǵ� ������ , WSAEnumNetworkEvents�� �� ����

		WSANETWORKEVENTS networkEvents;
		if (::WSAEnumNetworkEvents(sessions[index].socket, wsaEvents[index], &networkEvents) == SOCKET_ERROR)
			continue;


		//listener ���� üũ
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
				//Ŭ���Ʈ �����̶� ���� �̺�Ʈ�� ����������
				if (::WSAEventSelect(clientSocket, clientEvent, FD_READ | FD_WRITE | FD_CLOSE) == SOCKET_ERROR)
					return 0;
			}
		}

		//Client Session ���� üũ
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

		//FD_CLOSE ó��
		if (networkEvents.lNetworkEvents & FD_CLOSE) {

			//TODO: remove Socket
		}
	}



	//���� ����.
	::WSACleanup();
}

