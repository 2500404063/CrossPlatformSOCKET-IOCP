#include "sun_socket.h"

sun_socket::sun_socket()
{
#ifdef _MSC_VER
	if (WSAStartup(MAKEWORD(2, 2), &this->wsa) != 0) {
		perror("WSAStartup");
	}
#endif
#if defined(__GNUC__) || defined(__GNUG__)

#endif
}

sun_socket::~sun_socket()
{
#ifdef _MSC_VER
	WSACleanup();
#endif
#if defined(__GNUC__) || defined(__GNUG__)

#endif
}

int sun_socket::Listen(SOCKET* obj, const char* ip, unsigned int port, int backlog)
{
#ifdef _MSC_VER
	* obj = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddr_in eip;
	eip.sin_family = AF_INET;
	eip.sin_port = htons(port);
	inet_pton(AF_INET, ip, &eip.sin_addr.S_un.S_addr);
	::bind(*obj, (sockaddr*)& eip, sizeof(eip));
	return ::listen(*obj, backlog);
#endif
#if defined(__GNUC__) || defined(__GNUG__)
	* obj = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddr_in eip;
	eip.sin_family = PF_INET;
	eip.sin_port = htons(port);
	eip.sin_addr.s_addr = inet_addr(ip);
	bind(*obj, (sockaddr*)& eip, sizeof(eip));
	return ::listen(*obj, backlog);
#endif
}

SOCKET sun_socket::Accept(SOCKET obj, sockaddr_in* eip)
{
	socklen_t len = sizeof(sockaddr_in);
	SOCKET temp = accept(obj, (sockaddr*)eip, &len);
	return temp;
}

ssize_t sun_socket::Send(SOCKET obj, const char* data, int length)
{
	return send(obj, data, length, 0);
}

ssize_t sun_socket::Recv(SOCKET obj, char* buffer, int length)
{
	return recv(obj, buffer, length, 0);
}

int sun_socket::Stop(SOCKET obj)
{
#ifdef _MSC_VER
	return closesocket(obj);
#endif // _MSC_VER
#if defined(__GNUC__) || defined(__GNUG__)
	return close(obj);
#endif // defined
}

int sun_socket::Stop_R(SOCKET obj)
{
#ifdef _MSC_VER
	return shutdown(obj, SD_RECEIVE);
#endif // _MSC_VER
#if defined(__GNUC__) || defined(__GNUG__)
	return shutdown(obj, SHUT_RD);
#endif // defined
}

int sun_socket::Stop_W(SOCKET obj)
{
#ifdef _MSC_VER
	return shutdown(obj, SD_SEND);
#endif // _MSC_VER
#if defined(__GNUC__) || defined(__GNUG__)
	return shutdown(obj, SHUT_WR);
#endif // defined
}

int sun_socket::Stop_RW(SOCKET obj)
{
#ifdef _MSC_VER
	return shutdown(obj, SD_BOTH);
#endif // _MSC_VER
#if defined(__GNUC__) || defined(__GNUG__)
	return shutdown(obj, SHUT_RDWR);
#endif // defined
}

int sun_socket::Connect(SOCKET& sock, const char* ip, unsigned int port)
{
#ifdef _MSC_VER
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddr_in eip;
	eip.sin_family = AF_INET;
	eip.sin_port = htons(port);
	inet_pton(AF_INET, ip, &eip.sin_addr.S_un.S_addr);
	return connect(sock, (sockaddr*)& eip, sizeof(eip));
#endif // _MSC_VER
#if defined(__GNUC__) || defined(__GNUG__)
	sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddr_in eip;
	eip.sin_family = PF_INET;
	eip.sin_port = htons(port);
	eip.sin_addr.s_addr = inet_addr(ip);
	return connect(sock, (sockaddr*)& eip, sizeof(eip));
#endif // defined
}

void sun_socket::Getip(sockaddr_in eip, char* ip, int size_ip)
{
	inet_ntop(AF_INET, &eip.sin_addr, ip, size_ip);
}

void sun_socket::Getport(sockaddr_in eip, unsigned short* port)
{
	*port = ntohs(eip.sin_port);
}

std::string sun_socket::EncodeLenAStr(const char* instr, short area)
{
	char* lenstr = new char[area];//0000
	memset(lenstr, '*', area);
	size_t len = strlen(instr);//12
#ifdef _MSC_VER
	_itoa_s(len, lenstr, area, 10);//1200
#endif
#if defined(__GNUC__) || defined(__GNUG__)
	sprintf(lenstr,"%d",len);
#endif
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

std::vector<std::string> sun_socket::DecodeLenAStr(const char* instr, short area)
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