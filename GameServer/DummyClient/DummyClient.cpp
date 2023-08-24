//==============================================
// CLIENT CODE
//==============================================
// ����: ����ŷ ����
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
	//save("DummyClient.cpp", "ClientCodeFile.txt");
	cout << "Client" << endl;

	WSADATA wsData;
	if (::WSAStartup(MAKEWORD(2, 2), &wsData))
		return 0;

	SOCKET clientSocket = ::socket(AF_INET, SOCK_DGRAM, 0);
	// �ҹ��ڷ� �������� �������� �ְ�, �빮�ڴ� ���ӿ����� ��κ� �����ϴ°�
	if (clientSocket == INVALID_SOCKET) {
		HandleError("Socket");
		return 0;
	}

	

	//���� ���ҽ� ��ȯ
	closesocket(clientSocket);


	//���� ����.
	::WSACleanup();




}

