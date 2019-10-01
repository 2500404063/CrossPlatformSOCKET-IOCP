#ifndef sun_socket_define
#define sun_socket_define

#ifdef _MSC_VER
#if defined(_WIN64)
typedef long int ssize_t;
#else
typedef int ssize_t;
#endif
#endif

#ifdef _WIN64
typedef unsigned long long SOCKET;
#else
typedef unsigned int SOCKET;
#endif // _WIN64

#ifdef _MSC_VER
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>
#pragma comment(lib, "ws2_32.lib")
class __declspec(dllexport) sun_socket
{
private:
#ifdef _MSC_VER
	WSADATA wsa;
#endif
public:
	sun_socket();
	~sun_socket();
	int Listen(SOCKET *obj, const char *ip, unsigned int port, int backlog);
	SOCKET Accept(SOCKET obj, sockaddr_in *eip);
	ssize_t Send(SOCKET obj, const char *data, int length);
	ssize_t Recv(SOCKET obj, char *buffer, int length);
	int Stop(SOCKET obj);
	int Stop_R(SOCKET obj);
	int Stop_W(SOCKET obj);
	int Stop_RW(SOCKET obj);
	int Connect(SOCKET &sock, const char *ip, unsigned int port);
	void Getip(sockaddr_in eip, char *ip, int size_ip);
	void Getport(sockaddr_in eip, unsigned short *port);
};
#endif

#if defined(__GNUC__) || defined(__GNUG__)
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
class sun_socket
{
public:
	sun_socket();
	~sun_socket();
	int Listen(SOCKET *obj, const char *ip, unsigned int port, int backlog);
	SOCKET Accept(SOCKET obj, sockaddr_in *eip);
	ssize_t Send(SOCKET obj, const char *data, int length);
	ssize_t Recv(SOCKET obj, char *buffer, int length);
	int Stop(SOCKET obj);
	int Stop_R(SOCKET obj);
	int Stop_W(SOCKET obj);
	int Stop_RW(SOCKET obj);
	int Connect(SOCKET &sock, const char *ip, unsigned int port);
	void Getip(sockaddr_in eip, char *ip, int size_ip);
	void Getport(sockaddr_in eip, unsigned short *port);
};
#endif

#endif // !sun_socket
