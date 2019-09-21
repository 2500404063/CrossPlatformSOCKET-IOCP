#define _CRT_SECURE_NO_WARNINGS

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
	if (::listen(server, _Backlong) == 0) {
		std::thread accepting(&SunTCPServer::Accepting, this, server);
		accepting.detach();
		for (unsigned short i = 0; i < _Thread; i++)
		{
			std::thread th(&SunTCPServer::Worker, this);
			th.detach();
		}
		return server;
	}
	else {
		return -1;
	}
}

int SunTCPServer::Connect(const char* ip, int _Port, unsigned short _Thread)
{
	SOCKET client;
	client = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	sockaddr_in eip;
	eip.sin_family = AF_INET;
	inet_pton(AF_INET, ip, &eip.sin_addr.S_un.S_addr);
	eip.sin_port = htons(_Port);
	if (::connect(client, (sockaddr*)& eip, sizeof(eip)) != 0) {
		client = 0;
	}
	else {
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
			std::thread th(&SunTCPServer::Worker, this);
			th.detach();
		}
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
	if (info != 0) {
		shutdown(info->mSock, SD_BOTH);
		closesocket(info->mSock);
	}
}

void SunTCPServer::Close(int sock)
{
	shutdown(sock, SD_BOTH);
	closesocket(sock);
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

void SunTCPServer::Send(SOCKET sock, const char* data)
{
	PPER_IO_CONTEXT perio = new PER_IO_CONTEXT;
	perio->mSock = sock;
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
	while (true)
	{
		DWORD mTreansferred = 0;
		PPER_IO_CONTEXT perio;
		GetQueuedCompletionStatus(this->com, &mTreansferred, (PULONG_PTR)& perio, (LPOVERLAPPED*)& perio, INFINITE);
		perio = (PPER_IO_CONTEXT)CONTAINING_RECORD(perio, PER_IO_CONTEXT, mOverlapped);
		if (mTreansferred == 0)
		{
			callLeft(perio);
			delete perio;
			perio = 0;
		}
		else if (perio->mOperator == 1)
		{
			delete[]perio->mWsabuf.buf;
			delete perio;
			perio = 0;
		}
		else if (perio->mOperator == 0)
		{
			//perio->mWsabuf.len = mTreansferred;
			callReceive(perio);
			memset(perio->mBuffer, 0, sizeof(perio->mBuffer));
			//perio->mWsabuf.len = sizeof(perio->mBuffer);
			//This shows the size of buffer,if it's equal to 0, you will get nothing.

			DWORD BytesRecvd = 0;
			DWORD Flag = 0;
			//perio->mBuffer[mTreansferred] = 0;
			memset(&perio->mOverlapped, 0, sizeof(OVERLAPPED));
			WSARecv(perio->mSock, &perio->mWsabuf, 1, 0, &Flag, &perio->mOverlapped, 0);
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
		callAccept(perio);
		WSARecv(perio->mSock, &perio->mWsabuf, 1, &BytesRecvd, &Flag, &perio->mOverlapped, 0);
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
}
void SunTCPServer::Close(int sock)
{
	shutdown(sock, SHUT_RDWR);
	close(sock);
}
void SunTCPServer::Send(PINFO info, const char* data)
{
	send(info->mSock, data, strlen(data), 0);
}
void SunTCPServer::Send(int sock, const char* data)
{
	send(sock, data, strlen(data), 0);
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
							delete info;
						}
						else {
							perror("Recv Error");
						}
						delete info;
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
					delete info;
				}
			}
		}
	}
}
#endif


DataTools::DataTools()
{
}

DataTools::~DataTools()
{
}

std::string DataTools::EncodeLenAStr(const char* instr, short area = 8)
{
	char* lenstr = new char[area];//0000
	memset(lenstr, '*', area);
	size_t len = strlen(instr);//12
	_itoa(len, lenstr, 10);//1200
	std::string result;
	for (auto i = 0; i < area; i++)
	{
		if (lenstr[i] == '\0') {
			lenstr[i] = '*';
			result.push_back(lenstr[i]);
		}
		else {
			result.push_back(lenstr[i]);
		}
	}
	result.append(instr);//1200Hello...
	delete[]lenstr;
	return result;
}

std::vector<std::string> DataTools::DecodeLenAStr(const char* instr, short area)
{
	std::string str;
	str.append(instr);
	size_t datalen = str.size();
	size_t hasRead = 0;
	std::vector<std::string> result;
	while (hasRead < datalen) {
		std::string lenstr = str.substr(hasRead, area);
		for (size_t i = 0; i < lenstr.size(); i++)
		{
			if (lenstr[i] == '*') {
				if (i == 0) {
					lenstr = lenstr.substr(0, 1);
					break;
				}
				else {
					lenstr = lenstr.substr(0, i);
					break;
				}
			}
		}
		size_t len = atoi(lenstr.c_str());
		hasRead += (area);
		std::string end;
		end.append(str.substr(hasRead, len));
		//If there happens an error, I think that's because your buffer is not enough.
		hasRead += (len);
		result.push_back(end);
	}
	return result;
}

void DataTools::Log_Out(const char* filename, std::string level, std::string content)
{
	std::ofstream out(filename, std::ios::out | std::ios::app);
	if (out.is_open()) {
		time_t t;
		time(&t);
		tm* tp = localtime(&t);
		out << tp->tm_year << "-" << tp->tm_mon << "-" << tp->tm_mday << "  " << tp->tm_hour << ":" << tp->tm_min << ":" << tp->tm_sec;
		out << "   (" << level << ")";
		out << ":" << content << std::endl;
		out.close();
	}
}

void DataTools::Log_DEBUG(const char* filename, const char* content)
{
	Log_Out(filename, "DEBUG", content);
}
void DataTools::Log_WARN(const char* filename, const char* content)
{
	Log_Out(filename, "WARN", content);
}
void DataTools::Log_ERROR(const char* filename, const char* content)
{
	Log_Out(filename, "ERROR", content);
}
