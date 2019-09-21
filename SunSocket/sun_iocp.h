#pragma once

/*
Name: A async TCP pack across Platform
Author: Peng
Time:2019.9.21
Version:2.1
*/
#include "sun_socket.h"

#ifdef _MSC_VER
#include <string>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <thread>
#include <limits.h>
#pragma comment(lib,"ws2_32.lib")
#endif // _MSC_VER

#ifdef _MSC_VER
typedef struct _PER_IO {
	OVERLAPPED			mOverlapped;
	unsigned int		mSock;
	sockaddr_in			mEip;
	WSABUF				mWsabuf;
	unsigned short		mOperator;
} INFO, * PINFO;
enum IOCP_STATUS {
	Error = -1,
	Left = 0,
	Recv = 1,
	Send = 2
};
class sun_iocp : public sun_socket
{
public:
	sun_iocp();
	int WSASend(PINFO info);
	int WSARecv(PINFO info);
	int WSABind(PINFO info);
	void WSASetINFO(PINFO info, unsigned int sock, sockaddr_in eip, std::string* data, unsigned long buffer_size);
	void WSASetINFO(PINFO info, unsigned int sock, const char* ip, int port, std::string* data, unsigned long buffer_size);
	IOCP_STATUS WSAStatus(DWORD timeoutms);
	PINFO WSAGetLastInfo();
	char* WSAGetIP(PINFO info);
	SOCKET WSAGetSocket(PINFO info);
	unsigned short WSAGetPort(PINFO info);
	std::string WSAGetData(PINFO info);
	unsigned int WSAGetDataSize(PINFO info);
private:
	HANDLE com;
	PINFO LastInfo;
};
#endif // _MSC_VER