#ifndef SUN_EPOLL
#define SUN_EPOLL

#include "sun_socket.h"
#if defined(__GNUC__) || defined(__GNUG__)
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <fcntl.h>
#include <sys/types.h>
typedef struct _MoreInfo_
{
	unsigned int fd;
	sockaddr_in eip;
	char *buffer;	  //Used for recv
	ssize_t DataLen;   //Received data size;
	ssize_t BufferLen; //buffer size;

	char *Sendbuffer; //Used for send
	ssize_t SentLen;  //Sent data size;
	ssize_t SendLen;  //Sendbuffer size;

} MoreInfo, *PMoreInfo;

typedef epoll_event INFO, *PINFO;

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
	PMoreInfo more;
} WAITStatus, *PWAITStatus;
class sun_epoll : public sun_socket
{
public:
	sun_epoll();
	~sun_epoll();
	int WSASend(PMoreInfo more, std::string data, unsigned long len);
	int WSARecv(PMoreInfo more);
	int WSABind(PMoreInfo more);
	int WSAUnBind(PMoreInfo more);
	void WSASetINFO(PMoreInfo more, unsigned int sock, sockaddr_in eip, std::string *data, unsigned long buffer_size);
	void WSASetINFO(PMoreInfo more, unsigned int sock, const char *ip, int port, std::string *data, unsigned long buffer_size);
	void WSASetINFO(PMoreInfo more, std::string *data, unsigned long buffer_size);
	void WSAStatus(WAITStatus *out, unsigned int timeoutms);
	char *WSAGetIP(PMoreInfo more);
	SOCKET WSAGetSocket(PMoreInfo more);
	unsigned short WSAGetPort(PMoreInfo more);
	std::string WSAGetData(PMoreInfo more);
	unsigned int WSAGetDataSize(PMoreInfo more);

private:
	int com;
};
#endif

#endif