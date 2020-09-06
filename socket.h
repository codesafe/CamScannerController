#ifndef _SOCKET_
#define _SOCKET_

/*
	-- packet 구조 --
	packet size : 4 byte
	packet command : 4 byte
	packet data size : 5 byte
	packet data : data size 만큼
*/



#include <stdio.h>
#include <stdlib.h>

#ifdef __linux__
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr


#else

#include <Windows.h>
#include <winsock.h>

#endif

#include "predef.h"

struct SocketBuffer
{
	int		totalsize;		// 전체 길이
	int		currentsize;	// 진행중인 버퍼 위치 (송/수신 모두)
	char	buffer[SOCKET_BUFFER];
	SocketBuffer()
	{
		totalsize = -1;
		currentsize = 0;
		memset(buffer, 0, SOCKET_BUFFER);
	}
};

class Socket
{
public :
	Socket();
	~Socket();

	bool	init();
	void	uninit();

	bool	connect();
	void	closesocket();
	bool	update();

	bool	sendpacket(char *packet,int packetsize);		// 실제로 보내는것 아님
	bool	recvpacket(SocketBuffer *buffer);

private :
	void	senddone();
	void	recvdone();

	bool	initserver();
	bool	initclient();

	void	closeserver();
	void	closeclient();

	bool	updateserver();
	bool	updateclient();

#if WIN32
	SOCKET sock;
	SOCKET clisock;
#else
	int sock;
	int clisock;
#endif
	struct sockaddr_in server;
	struct sockaddr_in cli_addr;

	std::deque<SocketBuffer>	recvbufferlist;
	std::deque<SocketBuffer>	sendbufferlist;

	SocketBuffer sendbuffer;
	SocketBuffer recvbuffer;
};



#endif