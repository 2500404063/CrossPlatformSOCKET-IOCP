#include "sun_iocp.h"

//int sun_iocp::Start(const char* ip, int _Port, unsigned short _Backlong)
//{
//	SOCKET sock;
//	sock = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
//	sockaddr_in eip;
//	eip.sin_family = AF_INET;
//	inet_pton(AF_INET, ip, &eip.sin_addr.S_un.S_addr);
//	eip.sin_port = htons(_Port);
//	if (::bind(sock, (const sockaddr*)& eip, sizeof(eip)) == 0) {
//		if (listen(sock, _Backlong) == 0) {
//			return sock;
//		}
//		else {
//			perror("BIND");
//			return -1;
//		}
//	}
//	else {
//		perror("BIND");
//		return -1;
//	}
//	return 0;
//}

sun_iocp::sun_iocp()
{
#ifdef _MSC_VER
	this->com = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
#endif
}


int sun_iocp::WSASend(PINFO info)
{
	memset(&info->mOverlapped, 0, sizeof(OVERLAPPED));
	info->mOperator = 1;
	DWORD Transfered = 0;
	DWORD Flag = 0;
	::WSASend(info->mSock, &info->mWsabuf, 1, &Transfered, Flag, (LPWSAOVERLAPPED)& info->mOverlapped, 0);
	return Transfered;
}

int sun_iocp::WSARecv(PINFO info)
{
	memset(&info->mOverlapped, 0, sizeof(OVERLAPPED));
	info->mOperator = 2;
	DWORD Transfered = 0;
	DWORD Flag = 0;
	::WSARecv(info->mSock, &info->mWsabuf, 1, &Transfered, &Flag, (LPWSAOVERLAPPED)& info->mOverlapped, 0);
	return Transfered;
}

int sun_iocp::WSABind(PINFO info)
{
	memset(&info->mOverlapped, 0, sizeof(OVERLAPPED));
	CreateIoCompletionPort((HANDLE)info->mSock, this->com, (ULONG_PTR)& info, 0);
	return 0;
}

void sun_iocp::WSASetINFO(PINFO info, unsigned int sock, sockaddr_in eip, std::string* data, unsigned long buffer_size)
{
	info->mSock = sock;
	info->mEip = eip;
	info->mWsabuf.buf = &data->at(0);
	info->mWsabuf.len = buffer_size;
}

void sun_iocp::WSASetINFO(PINFO info, unsigned int sock, const char* ip, int port, std::string* data, unsigned long buffer_size)
{
	sockaddr_in eip;
	eip.sin_family = AF_INET;
	eip.sin_port = htons(port);
	inet_pton(AF_INET, ip, &eip.sin_addr.S_un.S_addr);
	info->mSock = sock;
	info->mEip = eip;
	info->mWsabuf.buf = &data->at(0);
	info->mWsabuf.len = buffer_size;
}

IOCP_STATUS sun_iocp::WSAStatus(DWORD timeoutms)
{
	DWORD mTreansferred = 0;
	PINFO perio;
	bool status = GetQueuedCompletionStatus(this->com, &mTreansferred, (PULONG_PTR)& perio, (LPOVERLAPPED*)& perio, timeoutms);
	if (status) {
		this->LastInfo = perio;
		if (mTreansferred == 0)
		{
			return IOCP_STATUS::Left;
		}
		else if (perio->mOperator == 1)
		{
			return IOCP_STATUS::Send;
		}
		else if (perio->mOperator == 2)
		{
			return IOCP_STATUS::Recv;
		}
	}
	return IOCP_STATUS::Error;
}

PINFO sun_iocp::WSAGetLastInfo()
{
	return this->LastInfo;
}

char* sun_iocp::WSAGetIP(PINFO info)
{
	char* ip = new char[16]{ 0 };
	InetNtopA(AF_INET, &info->mEip.sin_addr.S_un, ip, 16);
	return ip;
}

SOCKET sun_iocp::WSAGetSocket(PINFO info)
{
	return info->mSock;
}

unsigned short sun_iocp::WSAGetPort(PINFO info)
{
	return htons(info->mEip.sin_port);
}

std::string sun_iocp::WSAGetData(PINFO info)
{
	std::string temp;
	temp.append(info->mWsabuf.buf, info->mWsabuf.len);
	return temp;
}

unsigned int sun_iocp::WSAGetDataSize(PINFO info)
{
	return info->mWsabuf.len;
}
