#include "sun_iocp.h"
#include "sun_socket.h"
#include <iostream>
#include <thread>

using namespace std;
string buffer;
sun_iocp iocp;
SOCKET server;
void work();
void accepting();
int main() {
	buffer.resize(1024);
	iocp.Listen(&server, "0.0.0.0", 9998, 100);
	thread th(work);
	th.detach();
	thread th1(accepting);
	th1.detach();
	cin.get();
}
void accepting() {
	while (true)
	{
		PINFO info = new INFO;
		sockaddr_in eip;
		SOCKET client = iocp.Accept(server, &eip);
		iocp.SetINFO(info, client, eip, &buffer, buffer.size());
		iocp.WSABind(info);
		iocp.WSARecv(info);
	}
}
void work() {
	while (true)
	{
		switch (iocp.WSAStatus(-1))
		{
		case IOCP_STATUS::Left:
			cout << "Left:" << iocp.WSAGetIP(iocp.WSAGetLastInfo()) << endl;
			iocp.Stop_RW(iocp.WSAGetSocket(iocp.WSAGetLastInfo()));
			break;
		case IOCP_STATUS::Recv:
			cout << buffer << endl;
			buffer.resize(1024);
			iocp.WSARecv(iocp.WSAGetLastInfo());
			break;
		case IOCP_STATUS::Send:
			break;
		default:
			cout << "Error" << endl;
			break;
		}
	}
}