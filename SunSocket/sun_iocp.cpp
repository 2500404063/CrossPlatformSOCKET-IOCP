#include "sun_iocp.h"

#ifdef _MSC_VER
sun_iocp::sun_iocp()
{
	this->com = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
}
#endif
#ifdef _MSC_VER

int sun_iocp::WSASend(PINFO info, std::string data, unsigned long len)
{
	PINFO temp = new INFO;
	temp->mWsabuf.buf = &data.at(0);
	temp->mWsabuf.len = len;
	temp->mSock = info->mSock;

	memset(&temp->mOverlapped, 0, sizeof(OVERLAPPED));
	temp->mOperator = 1;
	DWORD Transfered = 0;
	DWORD Flag = 0;
	::WSASend(temp->mSock, &temp->mWsabuf, 1, &Transfered, Flag, (LPWSAOVERLAPPED)&temp->mOverlapped, 0);
	return Transfered;
}

int sun_iocp::WSARecv(PINFO info)
{
	memset(&info->mOverlapped, 0, sizeof(OVERLAPPED));
	info->mOperator = 2;
	DWORD Transfered = 0;
	DWORD Flag = 0;
	::WSARecv(info->mSock, &info->mWsabuf, 1, &Transfered, &Flag, (LPWSAOVERLAPPED)&info->mOverlapped, 0);
	return Transfered;
}

int sun_iocp::WSABind(PINFO info)
{
	memset(&info->mOverlapped, 0, sizeof(OVERLAPPED));
	CreateIoCompletionPort((HANDLE)info->mSock, this->com, (ULONG_PTR)&info, 0);
	return 0;
}

int sun_iocp::WSAUnBind(PINFO info)
{
	return 0;
}

void sun_iocp::WSASetINFO(PINFO info, unsigned int sock, sockaddr_in eip, std::string *data, unsigned long buffer_size)
{
	info->mSock = sock;
	info->mEip = eip;
	info->mWsabuf.buf = &data->at(0);
	info->mWsabuf.len = buffer_size;
}

void sun_iocp::WSASetINFO(PINFO info, unsigned int sock, const char *ip, int port, std::string *data, unsigned long buffer_size)
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

void sun_iocp::WSASetINFO(PINFO info, std::string *data, unsigned long buffer_size)
{
	info->mWsabuf.buf = &data->at(0);
	info->mWsabuf.len = buffer_size;
}

void sun_iocp::WSAStatus(WAITStatus *out, DWORD timeoutms)
{
	DWORD mTreansferred = 0;
	PINFO perio;
	bool status = GetQueuedCompletionStatus(this->com, &mTreansferred, (PULONG_PTR)&perio, (LPOVERLAPPED *)&perio, timeoutms);
	if (status)
	{
		if (mTreansferred == 0)
		{
			out->status = WAIT_STATUS::Left;
			out->info = perio;
			out->more = perio;
		}
		else if (perio->mOperator == 1)
		{
			perio->DataLen = mTreansferred;
			out->status = WAIT_STATUS::Send;
			delete perio;
			out->info = 0;
			out->more = 0;
		}
		else if (perio->mOperator == 2)
		{
			perio->DataLen = mTreansferred;
			out->status = WAIT_STATUS::Recv;
			out->info = perio;
			out->more = perio;
		}
		else
		{
			out->status = WAIT_STATUS::Error;
			out->info = perio;
			out->more = perio;
		}
	}
	else
	{
		out->status = WAIT_STATUS::Error;
		out->info = perio;
		out->more = perio;
	}
}

char *sun_iocp::WSAGetIP(PINFO info)
{
	char *ip = new char[16]{0};
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
	temp.append(info->mWsabuf.buf, info->DataLen);
	return temp;
}

unsigned int sun_iocp::WSAGetDataSize(PINFO info)
{
	return info->DataLen;
}

#endif