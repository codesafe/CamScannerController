
#include "predef.h"
#include "tcpsocket.h"
#include "logger.h"

TCP_Socket::TCP_Socket()
{

}
TCP_Socket::~TCP_Socket()
{

}

void TCP_Socket::init()
{
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1)
	{
		Logger::log(-1, "Could not create socket\n");
		return ;
	}
	std::string serveradd = MemDB::getInstance()->getValue("serveraddress");
	if (serveradd == "")
		serveradd = serveraddress;

	Logger::log(-1, "server address : %s\n", serveradd.c_str());

	memset((void*)&server, 0x00, sizeof(server));
	server.sin_addr.s_addr = inet_addr(serveradd.c_str());
	server.sin_family = AF_INET;
	server.sin_port = htons(SERVER_PORT);

	int flag = 1;
	int ret = setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag));
	if (ret == -1)
	{
		Logger::log(-1, "socket Setoption. Error!\n");
		return;
	}
}

bool TCP_Socket::connect()
{
	int err = ::connect(sock, (struct sockaddr*)&server, sizeof(server));
	if (err < 0)
	{
		Logger::log(-1, "connect failed. Error!\n");
		return false;
	}

	return true;
}

int TCP_Socket::update(char* buf)
{
	return 1;
}

int TCP_Socket::send(char* buf)
{
	return ::send(sock, buf, TCP_BUFFER, 0);
}

int TCP_Socket::recv(char* buf)
{
	return ::recv(sock, buf, TCP_BUFFER, 0);
}



