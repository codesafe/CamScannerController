#pragma once

#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr


class UDP_Socket
{
public :
	UDP_Socket();
	~UDP_Socket();

	void init(int camnum);
	int update(char *buf);
	void send(char *buf, int bufsize);


private :

	int	sock;
	struct sockaddr_in servAddr;
	struct sockaddr_in clntAddr;

};