#include "SunTCP.h"

#ifdef _MSC_VER

SunTCPServer::SunTCPServer()
{
#ifdef _MSC_VER
	WSADATA wsa;
	int back = WSAStartup(MAKEWORD(2, 2), &wsa);
	if (back != 0)
	{
		throw "WSAStartup Error";
	}
	this->com = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	if (com < 0)
	{
		throw "CreateIoCompletionPort Error";
	}
#endif // _MSC_VER
}

SunTCPServer::~SunTCPServer()
{

}

int SunTCPServer::Start(const char* ip, int _Port, unsigned short _Backlong, unsigned short _Thread, int MaxPlayer)
{
	SOCKET server;
	server = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	//server = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in eip;
	eip.sin_family = AF_INET;
	inet_pton(AF_INET, ip, &eip.sin_addr.S_un.S_addr);
	eip.sin_port = htons(_Port);
	::bind(server, (const sockaddr*)& eip, sizeof(eip));
	::listen(server, _Backlong);
	std::thread accepting(&SunTCPServer::Accepting, this, server);
	accepting.detach();
	for (unsigned short i = 0; i < _Thread; i++)
	{
		std::thread th(&SunTCPServer::Worker, this);
		th.detach();
	}
	if (server > 0)
		return server;
	else
		return -1;
}

int SunTCPServer::Connect(const char* ip, int _Port, unsigned short _Thread)
{
	SOCKET client;
	client = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	sockaddr_in eip;
	eip.sin_family = AF_INET;
	inet_pton(AF_INET, ip, &eip.sin_addr.S_un.S_addr);
	eip.sin_port = htons(_Port);
	::connect(client, (sockaddr*)& eip, sizeof(eip));

	PPER_IO_CONTEXT perio = new PER_IO_CONTEXT;
	memset(&perio->mOverlapped, 0, sizeof(OVERLAPPED));
	perio->mSock = client;
	perio->mEip = eip;
	perio->mWsabuf.buf = perio->mBuffer;
	perio->mWsabuf.len = sizeof(perio->mBuffer);
	perio->mOperator = 0;
	CreateIoCompletionPort((HANDLE)perio->mSock, this->com, (ULONG_PTR)& perio, 0);
	DWORD BytesRecvd = 0;
	DWORD Flag = 0;
	WSARecv(perio->mSock, &perio->mWsabuf, 1, &BytesRecvd, &Flag, &perio->mOverlapped, 0);
	callAccept(perio);
	for (size_t i = 0; i < _Thread; i++)
	{
		std::thread th(&SunTCPServer::ClientWorker, this);
		th.detach();
	}
	return client;
}

void SunTCPServer::Match(Callback Accept, Callback Receive, Callback Left)
{
	this->callAccept = Accept;
	this->callReceive = Receive;
	this->callLeft = Left;
}

char* SunTCPServer::GetIP(PINFO info)
{
	char* ip = new char[16]{ 0 };
	InetNtopA(AF_INET, &info->mEip.sin_addr.S_un, ip, 16);
	return ip;
}

PINFO SunTCPServer::GetClent(PINFO info)
{
	return info;
}

unsigned short SunTCPServer::GetPort(PINFO info)
{
	return htons(info->mEip.sin_port);
}

char* SunTCPServer::GetData(PINFO info)
{
	return info->mWsabuf.buf;
}

unsigned int SunTCPServer::GetDataSize(PINFO info)
{
	return info->mWsabuf.len;
}

void SunTCPServer::Close(PINFO info)
{
	shutdown(info->mSock, SD_BOTH);
	closesocket(info->mSock);
	delete info;
}

void SunTCPServer::Send(PINFO info, const char* data)
{
	PPER_IO_CONTEXT perio = new PER_IO_CONTEXT;
	perio->mSock = info->mSock;
	memset(&perio->mOverlapped, 0, sizeof(OVERLAPPED));
	size_t len = strlen(data);
	char* all = new char[len + 1]{ 0 };
	strcpy_s(all, len + 1, data);
	perio->mWsabuf.buf = all;
	perio->mWsabuf.len = len;
	perio->mOperator = 1;
	DWORD sent = 0;
	WSASend(perio->mSock, &perio->mWsabuf, 1, &sent, 0, (LPWSAOVERLAPPED)& perio->mOverlapped, 0);
}

void SunTCPServer::Worker()
{
	DWORD mTreansferred = 0;
	PPER_IO_CONTEXT perio;
	while (true)
	{
		GetQueuedCompletionStatus(this->com, &mTreansferred, (PULONG_PTR)& perio, (LPOVERLAPPED*)& perio, INFINITE);
		perio = (PPER_IO_CONTEXT)CONTAINING_RECORD(perio, PER_IO_CONTEXT, mOverlapped);
		if (mTreansferred == 0)
		{
			callLeft(perio);
		}
		else if (perio->mOperator == 1)
		{
			delete perio->mWsabuf.buf;
			delete perio;
		}
		else if (perio->mOperator == 0)
		{
			perio->mWsabuf.len = mTreansferred;
			DWORD BytesRecvd = 0;
			DWORD Flag = 0;
			WSARecv(perio->mSock, &perio->mWsabuf, 1, &BytesRecvd, &Flag, &perio->mOverlapped, 0);
			callReceive(perio);
		}
	}
}

void SunTCPServer::Accepting(unsigned int server)
{
	sockaddr_in eip;
	int client;
	int len = sizeof(eip);
	while (true)
	{
		client = accept(server, (sockaddr*)& eip, &len);
		PPER_IO_CONTEXT perio = new PER_IO_CONTEXT;
		memset(&perio->mOverlapped, 0, sizeof(OVERLAPPED));
		perio->mSock = client;
		perio->mEip = eip;
		perio->mWsabuf.buf = perio->mBuffer;
		perio->mWsabuf.len = sizeof(perio->mBuffer);
		perio->mOperator = 0;
		CreateIoCompletionPort((HANDLE)perio->mSock, this->com, (ULONG_PTR)& perio, 0);
		DWORD BytesRecvd = 0;
		DWORD Flag = 0;
		WSARecv(perio->mSock, &perio->mWsabuf, 1, &BytesRecvd, &Flag, &perio->mOverlapped, 0);
		callAccept(perio);
	}
}

void SunTCPServer::ClientWorker()
{
	DWORD mTreansferred = 0;
	PPER_IO_CONTEXT perio;
	while (true)
	{
		GetQueuedCompletionStatus(this->com, &mTreansferred, (PULONG_PTR)& perio, (LPOVERLAPPED*)& perio, INFINITE);
		perio = (PPER_IO_CONTEXT)CONTAINING_RECORD(perio, PER_IO_CONTEXT, mOverlapped);
		if (mTreansferred == 0)
		{
			callLeft(perio);
		}
		else if (perio->mOperator == 1)
		{
			delete perio->mWsabuf.buf;
			delete perio;
		}
		else if (perio->mOperator == 0)
		{
			perio->mWsabuf.len = mTreansferred;
			DWORD BytesRecvd = 0;
			DWORD Flag = 0;
			WSARecv(perio->mSock, &perio->mWsabuf, 1, &BytesRecvd, &Flag, &perio->mOverlapped, 0);
			callReceive(perio);
		}

	}
}

#endif // _MSC_VER


#if defined (__GNUC__) || defined(__GNUG__)


SunTCPServer::SunTCPServer()
{

}

SunTCPServer::~SunTCPServer()
{

}

void setnonblocking(int sock)
{
	int opts;
	opts = fcntl(sock, F_GETFL, 0);
	fcntl(sock, F_SETFL, opts | O_NONBLOCK);
}
int SunTCPServer::Start(const char* ip, uint16_t _Port, unsigned short _Backlong, unsigned short _Thread, int MaxPlayer)
{
	int server;
	server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddr_in eip;
	eip.sin_family = AF_INET;
	eip.sin_addr.s_addr = inet_addr(ip);
	eip.sin_port = htons(_Port);
	setnonblocking(server);
	::bind(server, (const sockaddr*)& eip, sizeof(eip));
	::listen(server, _Backlong);
	this->epfd = epoll_create1(0);
	if (epfd < 0)
	{
		throw "Create Epoll Badly";
	}
	epoll_event event;
	event.data.fd = server;
	event.events = EPOLLIN | EPOLLET;
	epoll_ctl(this->epfd, EPOLL_CTL_ADD, server, &event);

	this->IsClient = false;
	for (unsigned short i = 0; i < _Thread; i++)
	{
		std::thread th(&SunTCPServer::Worker, this, server);
		th.detach();
	}
	this->MaxPlayer = MaxPlayer;
	if (server > 0)
		return server;
	else
		return -1;
}
int SunTCPServer::Connect(const char* ip, uint16_t _Port, unsigned short _Thread)
{
	int server;
	server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddr_in eip;
	eip.sin_family = AF_INET;
	eip.sin_addr.s_addr = inet_addr(ip);
	eip.sin_port = htons(_Port);
	setnonblocking(server);
	connect(server, (sockaddr*)& eip, sizeof(eip));
	this->epfd = epoll_create(1024);
	if (epfd < 0)
	{
		throw "Create Epoll Badly";
	}
	PINFO info = new INFO;
	info->mEip = eip;
	info->mSock = server;
	info->mWsabuf.buf = info->mBuffer;
	info->mWsabuf.len = sizeof(info->mBuffer) / sizeof(char);

	epoll_event event;
	event.data.fd = info->mSock;
	event.events = EPOLLIN | EPOLLET;

	epoll_ctl(this->epfd, EPOLL_CTL_ADD, server, &event);

	this->IsClient = true;
	callAccept(info);
	for (unsigned short i = 0; i < _Thread; i++)
	{
		std::thread th(&SunTCPServer::Worker, this, server);
		th.detach();
	}
}
void SunTCPServer::Match(Callback Accept, Callback Receive, Callback Left)
{
	this->callAccept = Accept;
	this->callReceive = Receive;
	this->callLeft = Left;
}
char* SunTCPServer::GetIP(PINFO info)
{
	char* ip = new char[16]{ 0 };
	ip = inet_ntoa(info->mEip.sin_addr);
	return ip;
}
PINFO SunTCPServer::GetClent(PINFO info)
{
	return info;
}
unsigned short SunTCPServer::GetPort(PINFO info)
{
	return htons(info->mEip.sin_port);
}
char* SunTCPServer::GetData(PINFO info)
{
	return info->mWsabuf.buf;
}
unsigned int SunTCPServer::GetDataSize(PINFO info)
{
	return info->mWsabuf.len;
}
void SunTCPServer::Close(PINFO info)
{
	shutdown(info->mSock, SHUT_RDWR);
	close(info->mSock);
	delete info;
}
void SunTCPServer::Send(PINFO info, const char* data)
{
	send(info->mSock, data, strlen(data), 0);
}
void SunTCPServer::Worker(int server)
{
	epoll_event* events = new epoll_event[this->MaxPlayer];
	while (true)
	{
		int amount = epoll_wait(this->epfd, events, this->MaxPlayer, -1);
		if (amount < 0) {
			perror("Epoll_Wait Error");
		}
		else {
			for (long int i = 0; i <= amount; i++)
			{
				if (events[i].data.fd == server && events[i].events == EPOLLIN)
				{
					if (this->IsClient == false) {
						for (;;) {
							sockaddr_in eip;
							socklen_t len = sizeof(sockaddr_in);
							int client;
							client = accept(server, (sockaddr*)& eip, &len);
							if (client > 0) {
								setnonblocking(client);
								PINFO info = new INFO;
								info->mEip = eip;
								info->mSock = client;
								info->mWsabuf.buf = info->mBuffer;
								info->mWsabuf.len = sizeof(info->mBuffer) / sizeof(char);

								epoll_event event;
								event.data.fd = info->mSock;
								event.events = EPOLLIN | EPOLLET;
								epoll_ctl(this->epfd, EPOLL_CTL_ADD, info->mSock, &event);
								callAccept(info);
								delete info;
							}
							else {
								break;
							}
						}
					}
					else {
						PINFO info = new INFO;
						info->mWsabuf.len = sizeof(info->mWsabuf.buf);
						info->mWsabuf.buf = info->mBuffer;
						info->mSock = events[i].data.fd;
						ssize_t len = recv(events[i].data.fd, info->mWsabuf.buf, info->mWsabuf.len, 0);
						if (len > 0)
						{
							callReceive(info);
						}
						else if (len == 0)
						{
							epoll_ctl(this->epfd, EPOLL_CTL_DEL, events[i].data.fd, &events[i]);
							callLeft(info);
						}
						else {
							perror("Recv Error");
						}
					}
				}
				else if (events[i].events == EPOLLIN)
				{
					PINFO info = new INFO;
					info->mWsabuf.len = sizeof(info->mWsabuf.buf);
					info->mWsabuf.buf = info->mBuffer;
					info->mSock = events[i].data.fd;
					ssize_t len = recv(events[i].data.fd, info->mWsabuf.buf, info->mWsabuf.len, 0);
					if (len > 0)
					{
						callReceive(info);
					}
					else if (len == 0)
					{
						epoll_ctl(this->epfd, EPOLL_CTL_DEL, events[i].data.fd, &events[i]);
						callLeft(info);
					}
					else {
						perror("Recv Error");
					}
					//delete info;
					//The produre of releasing has put into void Close();
				}
			}
		}
	}
}
#endif
