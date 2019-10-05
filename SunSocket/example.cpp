//#include "sun_epoll.h"
#include "sun_iocp.h"
#include "sun_socket.h"
#include <iostream>
#include <thread>

using namespace std;
sun_iocp epoll;
SOCKET server;
void work();
void accepting();
int main()
{
	int sock = epoll.Listen(&server, "0.0.0.0", 9998, 100);
	thread th(work);
	th.detach();
	thread th1(accepting);
	th1.detach();
	cin.get();
}
void accepting()
{
	while (true)
	{
		PMoreInfo more = new MoreInfo;
		sockaddr_in eip;
		SOCKET client = epoll.Accept(server, &eip);
		epoll.WSASetINFO(more, client, eip, 1024);
		epoll.WSABind(more);
		epoll.WSARecv(more);
	}
}
void work()
{
	while (true)
	{
		WAITStatus status;
		epoll.WSAStatus(&status, 500);
		switch (status.status)
		{
		case WAIT_STATUS::Left:
			cout << "Left:" << epoll.WSAGetIP(status.more) << endl;
			epoll.Stop_RW(epoll.WSAGetSocket(status.more));
			epoll.WSAUnBind(status.more);
			delete status.more;
			break;
		case WAIT_STATUS::Recv:
			cout << epoll.WSAGetData(status.more) << endl;
			epoll.WSASend(status.more, "Hello", 5);

			epoll.WSASetINFO(status.more, 1024);
			epoll.WSARecv(status.more);
			break;
		case WAIT_STATUS::Send:
			break;
		case WAIT_STATUS::Error:
			break;
		default:
			break;
		}
	}
}