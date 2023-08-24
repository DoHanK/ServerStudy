//==============================================
// CLIENT CODE
//==============================================
// ����: UDP �ڵ� �ۼ�
//==============================================


#include "pch.h"
#include <iostream>

//����ũ�μ���Ʈ���� �����ϴ� ���� ���̺귯��
#include <WinSock2.h>
#include <MSWSock.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")



int main()
{
	//save("DummyClient.cpp", "ClientCodeFile.txt");

	//���� �ʱ�ȭ (WS2_32 ���̺귯�� �ʱ�ȭ)
	//���� ������ wsaData�� ä����

	WSADATA wsData;
	if (::WSAStartup(MAKEWORD(2, 2), &wsData))
		return 0;

	//af(Address Family)->  (AF_INET) IPv4 , (AF_INET6) IPv6
	//tpye -> tcp�� , udp�ĸ� �����ϴ°�
	//protocol ���� 0�� �ָ� �˾Ƽ� ������
	//return :descriptor �����͸� �������ش� ������ �˸��
	SOCKET clientSocket = ::socket(AF_INET, SOCK_STREAM, 0);
	// �ҹ��ڷ� �������� �������� �ְ�, �빮�ڴ� ���ӿ����� ��κ� �����ϴ°�
	if (clientSocket == INVALID_SOCKET) {

		int32 errCode = ::WSAGetLastError();
		cout << "SocketErrorCode" << endl;
		return 0;
	}

	SOCKADDR_IN	serverAddr;//IPv4
	::memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	//serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); //derprecated �ϴ�
	::inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
	serverAddr.sin_port = htons(7777);
	

	////////////////////////////////////////////////////////////////////
	//host to network short 
	//���ڸ� �����Ҷ� �ΰ��� ���
	//LIttle-Endian VS Big-Endian
	// ex) 0x12345678
	//low  78 56 34 12   high ->��Ʋ �����	 - >������ ��Ʋ ������� �����.
	//low 12 34 56 78    high -> �� �����   ->��Ʈ��ũ���� ä���� ���
	/////////////////////////////////////////////////////////////////////


	if(::connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)))
	{
		int32 errCode = ::WSAGetLastError();
		cout << "SocketErrorCode" << endl;
		return 0;
	}



		//==============================================
		// ���� ����! �������� ������ �ۼ����� �����ϴ�.!
		//===============================================
	while (true) {

		//==============================================
		// ��� �ڵ� �ۼ�
		//===============================================


		//==============================================
		//Ŭ�� - > ���� ������ ���� �ڵ�
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
		// ���� - >Ŭ�� ������ ���ú� �ڵ�
		//===============================================

 
		//���� �������� ũ�⸦ ��ȯ���� recv �����ʹ� -1�� ��ȯ�ϸ�.������ ����
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


	//���� ���ҽ� ��ȯ
	closesocket(clientSocket);


	//���� ����.
	::WSACleanup();




}

