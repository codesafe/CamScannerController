#pragma once

#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <netinet/tcp.h>


class TCP_Socket
{
public:
	TCP_Socket();
	~TCP_Socket();

	void init();
	bool connect();
	int update(char* buf);
	int send(char* buf);
	int recv(char* buf);

private:

	int	sock;
	struct sockaddr_in server;
	struct sockaddr_in clntAddr;

};
