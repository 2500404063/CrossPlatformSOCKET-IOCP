#ifndef SUN_IOCP
#define SUN_IOCP

#include "sun_socket.h"
#ifdef _MSC_VER
#include <string>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <thread>
#include <limits.h>
#pragma comment(lib, "ws2_32.lib")
#endif // _MSC_VER

#ifdef _MSC_VER

typedef struct _PER_IO
{
	OVERLAPPED mOverlapped;
	unsigned int mSock;
	sockaddr_in mEip;
	WSABUF mWsabuf;
	ssize_t DataLen;
	unsigned short mOperator;
} INFO, *PINFO, *PMoreInfo, MoreInfo;
enum WAIT_STATUS
{
	Error = 0,
	Left = 1,
	Recv = 2,
	Send = 3
};
typedef struct _WAIT_STATUS
{
	WAIT_STATUS status = WAIT_STATUS::Error;
	PINFO info;
	PINFO more = info;
} WAITStatus, *PWAITStatus;
class sun_iocp : public sun_socket
{
public:
	sun_iocp();
	int WSASend(PINFO info, std::string data, unsigned long len);
	int WSARecv(PINFO info);
	int WSABind(PINFO info);
	int WSAUnBind(PINFO info);
	void WSASetINFO(PINFO info, unsigned int sock, sockaddr_in eip, std::string *data, unsigned long buffer_size);
	void WSASetINFO(PINFO info, unsigned int sock, const char *ip, int port, std::string *data, unsigned long buffer_size);
	void WSASetINFO(PINFO info, std::string *data, unsigned long buffer_size);
	void WSAStatus(WAITStatus *out, DWORD timeoutms);
	char *WSAGetIP(PINFO info);
	SOCKET WSAGetSocket(PINFO info);
	unsigned short WSAGetPort(PINFO info);
	std::string WSAGetData(PINFO info);
	unsigned int WSAGetDataSize(PINFO info);

private:
	HANDLE com;
};
#endif // _MSC_VER
#endif