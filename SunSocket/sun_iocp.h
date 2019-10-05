#ifndef SUN_IOCP
#define SUN_IOCP

#if defined(_MSC_VER)

#include "sun_socket.h"
#include <string>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <thread>
#include <limits.h>
#pragma comment(lib, "ws2_32.lib")

struct _PER_IO
{
	OVERLAPPED mOverlapped;
	unsigned int mSock;
	sockaddr_in mEip;
	std::string RecvBuffer;
	std::string SendBuffer;
	WSABUF mWsabuf;
	ssize_t DataLen;
	unsigned short mOperator;
};
typedef _PER_IO INFO, * PINFO, * PMoreInfo, MoreInfo;
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
	PMoreInfo more = info;
} WAITStatus, * PWAITStatus;
class sun_iocp : public sun_socket
{
public:
	sun_iocp();
	int WSASend(PMoreInfo info, std::string data, unsigned long len);
	int WSASend(SOCKET sock, std::string data, unsigned long len);
	int WSARecv(PMoreInfo info);
	int WSABind(PMoreInfo info);
	int WSAUnBind(PMoreInfo info);
	void WSASetINFO(PMoreInfo info, unsigned int sock, sockaddr_in eip, unsigned long buffer_size);
	void WSASetINFO(PMoreInfo info, unsigned int sock, const char* ip, int port, unsigned long buffer_size);
	void WSASetINFO(PMoreInfo info, unsigned long buffer_size);
	void WSAStatus(WAITStatus* out, DWORD timeoutms);
	char* WSAGetIP(PMoreInfo info);
	SOCKET WSAGetSocket(PMoreInfo info);
	unsigned short WSAGetPort(PMoreInfo info);
	std::string WSAGetData(PMoreInfo info);
	unsigned int WSAGetDataSize(PMoreInfo info);

private:
	HANDLE com;
};
#endif // _MSC_VER

#endif