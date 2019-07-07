#pragma once

#include <iostream>

#include <string>
#ifdef _MSC_VER
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <thread>
#pragma comment(lib,"ws2_32.lib")
#endif // _MSC_VER

#if defined (__GNUC__) || defined(__GNUG__)
#include <thread>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <limits.h>
#endif

#ifdef _MSC_VER
typedef struct _PER_IO_CONTEXT {
	OVERLAPPED			mOverlapped;
	unsigned int		mSock;
	sockaddr_in			mEip;
	WSABUF				mWsabuf;
	char				mBuffer[0x10000]{ 0 };
	unsigned short		mOperator;

} PER_IO_CONTEXT, * PPER_IO_CONTEXT, INFO, * PINFO;

typedef void(*Callback)(PINFO info);

class SunTCPServer
{
public:
	SunTCPServer();
	~SunTCPServer();
	int Start(const char* ip, int _Port, unsigned short _Backlong, unsigned short _Thread, int MaxPlayer);
	int Connect(const char* ip, int _Port, unsigned short _Thread);
	void Match(Callback Accept, Callback Receive, Callback Left);
	char* GetIP(PINFO info);
	PINFO GetClent(PINFO info);
	unsigned short GetPort(PINFO info);
	char* GetData(PINFO info);
	unsigned int GetDataSize(PINFO info);
	void Close(PINFO info);
	void Send(PINFO info, const char* data);
private:
	void* com;
	void Worker();
	void ClientWorker();
	void Accepting(unsigned int server);
	Callback callAccept = 0, callReceive = 0, callLeft = 0;
};

#endif // _MSC_VER

#if defined (__GNUC__) || defined(__GNUG__)

typedef struct _WSABUF {
	char* buf;
	unsigned int len;
}WSABUF, * PWSABUF;

typedef struct _PER_IO_CONTEXT {
	unsigned int		mSock;
	sockaddr_in			mEip;
	WSABUF				mWsabuf;
	char				mBuffer[0x10000]{ 0 };
	unsigned short		mOperator;

} PER_IO_CONTEXT, * PPER_IO_CONTEXT, INFO, * PINFO;

typedef void(*Callback)(PINFO info);

class SunTCPServer
{
public:
	SunTCPServer();
	~SunTCPServer();
	int Start(const char* ip, uint16_t _Port, unsigned short _Backlong, unsigned short _Thread, int MaxPlayer);
	int Connect(const char* ip, uint16_t _Port, unsigned short _Thread);
	void Match(Callback Accept, Callback Receive, Callback Left);
	char* GetIP(PINFO info);
	PINFO GetClent(PINFO info);
	unsigned short GetPort(PINFO info);
	char* GetData(PINFO info);
	unsigned int GetDataSize(PINFO info);
	void Close(PINFO info);
	void Send(PINFO info, const char* data);
private:
	int epfd = 0;
	int MaxPlayer = 0x10000;
	void Worker(int server);
	Callback callAccept = 0, callReceive = 0, callLeft = 0;
	bool IsClient = false;
};

#endif
