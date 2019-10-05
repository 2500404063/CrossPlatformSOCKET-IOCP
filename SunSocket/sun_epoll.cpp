#include "sun_epoll.h"

#if defined(__GNUC__) || defined(__GNUG__)

void setnonblocking(int sock)
{
	int opts;
	opts = fcntl(sock, F_GETFL, 0);
	fcntl(sock, F_SETFL, opts | O_NONBLOCK);
}

void setreuseaddr(int sock)
{
	int opt;
	opt = 1;
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(&opt)) < 0)
	{
		perror("setsockopt");
		exit(1);
	}
}

sun_epoll::sun_epoll()
{
	this->com = epoll_create1(0);
	if (this->com <= 0)
	{
		perror("CREATE EPOLL");
	}
}
sun_epoll::~sun_epoll()
{
	close(this->com);
}

int sun_epoll::WSASend(PMoreInfo more, std::string data, unsigned long len)
{
	INFO info;
	info.data.ptr = more;
	info.events = EPOLLOUT | EPOLLRDHUP | EPOLLHUP | EPOLLET;
	epoll_ctl(this->com, EPOLL_CTL_MOD, more->fd, &info);
	more->SendBuffer.append(data);
	more->SendBufferLen = len;
	ssize_t sentlen = send(more->fd, &more->SendBuffer.at(0), more->SendBufferLen, 0);
	more->SentLen = sentlen;
	return sentlen;
}
int sun_epoll::WSASend(SOCKET sock, std::string data, unsigned long len)
{
	INFO info;
	info.data.fd = sock; //������Ҫ���һ��
	info.events = EPOLLOUT | EPOLLRDHUP | EPOLLHUP | EPOLLET;
	epoll_ctl(this->com, EPOLL_CTL_MOD, info.data.fd, &info);
	ssize_t sentlen = send(info.data.fd, &data.at(0), len, 0); //�ڶ����������Ƿ���Ҫȫ��?
	return sentlen;
}
int sun_epoll::WSARecv(PMoreInfo more)
{
	INFO info;
	info.data.ptr = more;
	info.events = EPOLLIN | EPOLLRDHUP | EPOLLHUP | EPOLLET;
	epoll_ctl(this->com, EPOLL_CTL_MOD, more->fd, &info);
}
int sun_epoll::WSABind(PMoreInfo more)
{
	INFO info;
	info.data.ptr = more;
	setnonblocking(more->fd);
	epoll_ctl(this->com, EPOLL_CTL_ADD, more->fd, &info);
}
int sun_epoll::WSAUnBind(PMoreInfo more)
{
	INFO info;
	info.data.ptr = more;
	epoll_ctl(this->com, EPOLL_CTL_DEL, more->fd, &info);
}
void sun_epoll::WSASetINFO(PMoreInfo more, unsigned int sock, sockaddr_in eip, unsigned long buffer_size)
{
	more->fd = sock;
	more->RecvBuffer.clear();
	more->RecvBuffer.resize(buffer_size);
	more->RecvBufferLen = buffer_size;
	more->eip = eip;
}
void sun_epoll::WSASetINFO(PMoreInfo more, unsigned int sock, const char *ip, int port, unsigned long buffer_size)
{
	more->fd = sock;
	more->RecvBuffer.clear();
	more->RecvBuffer.resize(buffer_size);
	more->RecvBufferLen = buffer_size;
	more->eip.sin_family = AF_INET;
	inet_pton(AF_INET, ip, &more->eip.sin_addr.s_addr);
	more->eip.sin_port = htons(port);
}
void sun_epoll::WSASetINFO(PMoreInfo more, unsigned long buffer_size)
{
	more->RecvBuffer.clear();
	more->RecvBuffer.resize(buffer_size);
	more->RecvBufferLen = buffer_size;
}
void sun_epoll::WSAStatus(WAITStatus *out, unsigned int timeoutms)
{
	epoll_event events[1];
	int amount = epoll_wait(this->com, events, 1, timeoutms);
	if (amount <= 0 && errno == EINTR)
	{
		out->status = WAIT_STATUS::Error;
		out->info = &events[0];
	}
	else
	{
		for (size_t i = 0; i < amount; i++)
		{
			if (events[i].events & EPOLLHUP)
			{
				out->status = WAIT_STATUS::Left;
				out->info = &events[i];
				out->more = static_cast<PMoreInfo>(out->info->data.ptr);
			}
			else if (events[i].events & EPOLLRDHUP)
			{
				out->status = WAIT_STATUS::Left;
				out->info = &events[i];
				out->more = static_cast<PMoreInfo>(out->info->data.ptr);
			}
			else if (events[i].events & EPOLLIN)
			{
				PMoreInfo more = (PMoreInfo)events[i].data.ptr;
				unsigned long len = recv(more->fd, &more->RecvBuffer.at(0), more->RecvBufferLen, 0);
				more->DataLen = len;

				out->status = WAIT_STATUS::Recv;
				out->info = &events[i];
				out->more = static_cast<PMoreInfo>(out->info->data.ptr);
			}
			else if (events[i].events & EPOLLOUT)
			{
				PMoreInfo more = (PMoreInfo)events[i].data.ptr;
				ssize_t len = send(more->fd, &more->SendBuffer.at(0) + more->SentLen, more->SendBufferLen - more->SentLen, 0);
				more->SentLen += len;

				out->status = WAIT_STATUS::Send;
				out->info = &events[i];
				out->more = static_cast<PMoreInfo>(out->info->data.ptr);
			}
			else
			{
				out->status = WAIT_STATUS::Error;
				out->info = &events[i];
				out->more = static_cast<PMoreInfo>(out->info->data.ptr);
			}
		}
	}
}
char *sun_epoll::WSAGetIP(PMoreInfo more)
{
	char *ip;
	ip = inet_ntoa(more->eip.sin_addr);
	return ip;
}
SOCKET sun_epoll::WSAGetSocket(PMoreInfo more)
{
	return more->fd;
}
unsigned short sun_epoll::WSAGetPort(PMoreInfo more)
{
	return htons(more->eip.sin_port);
}
std::string sun_epoll::WSAGetData(PMoreInfo more)
{
	std::string temp;
	temp.append(&more->RecvBuffer.at(0), more->DataLen);
	return temp;
}
unsigned int sun_epoll::WSAGetDataSize(PMoreInfo more)
{
	return more->DataLen;
}

#endif