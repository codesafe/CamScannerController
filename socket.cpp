
#include <netinet/tcp.h>
#include "socket.h"
#include "logger.h"

#ifdef WIN32
#pragma comment(lib, "ws2_32.lib")
#endif


//#define USE_NONEBLOCK 1

Socket::Socket()
{
	sock = -1;
	clisock = -1;
}

Socket::~Socket()
{

}

bool	Socket::init()
{
#ifdef I_AM_SERVER
	return initserver();
#else

	return initclient();
#endif



	return true;
}

bool	Socket::initserver()
{
#ifdef I_AM_SERVER
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1)
	{
		Logger::log(LOG_ERR, "Could not create socket\n");
		return false;
	}

	memset((void *)&server, 0x00, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(SERVER_PORT);

	int status = bind(sock, (struct sockaddr*)&server, sizeof(struct sockaddr_in));
	if (status == SOCKET_ERROR)
		Logger::log(LOG_INFO, "Bind Error\n");

	listen(sock, 5);
	Logger::log(LOG_INFO, "Socket created!\n");
#endif
	return true;
}

bool	Socket::initclient()
{
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1)
	{
		Logger::log(CLIENT_LOG_ERR, "Could not create socket\n");
		return false;
	}
	std::string serveradd = MemDB::getInstance()->getValue("serveraddress");
	if (serveradd == "")
		serveradd = SERVER_ADD;

	Logger::log(CLIENT_LOG_INFO, "server address : %s\n", serveradd.c_str());

	memset((void *)&server, 0x00, sizeof(server));
	server.sin_addr.s_addr = inet_addr(serveradd.c_str());
	server.sin_family = AF_INET;
	server.sin_port = htons(SERVER_PORT);

	int flag = 1;
	int ret = setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag));
	if (ret == -1)
	{
		Logger::log(CLIENT_LOG_ERR, "socket Setoption. Error!\n");
		return false;
	}

	return true;
}


void	Socket::uninit()
{
	closesocket();
	recvbufferlist.clear();
	sendbufferlist.clear();
}

bool	Socket::connect()
{

	//Connect to server
	int err = ::connect(sock, (struct sockaddr *)&server, sizeof(server));
	if ( err < 0)
	{
		Logger::log(CLIENT_LOG_ERR, "connect failed. Error!\n");
		return false;
	}

	//set_nonblock
#ifdef USE_NONEBLOCK
	#ifdef __linux__
		int flags = fcntl(sock, F_GETFL, 0);
		//assert(flags != -1);
		if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) < 0) return false;
	#else
		unsigned long arg = 1;
		if( ioctlsocket(sock, FIONBIO, &arg) != 0) return false;
	#endif
#endif
	


	return true;
}

void	Socket::closesocket()
{
#ifdef I_AM_SERVER
	closeserver();
#else

	closeclient();
#endif
}

void	Socket::closeserver()
{
	if (sock > 0)
	{
#if WIN32
		::closesocket(sock);
#else
		close(sock);
#endif
		sock = -1;
	}

	if (clisock > 0)
	{
#if WIN32
		::closesocket(clisock);
#else
		close(clisock);
#endif
		clisock = -1;
	}
}

void	Socket::closeclient()
{
	if (sock == -1) return;
#if WIN32
	::closesocket(sock);
#else
	close(sock);
#endif
	sock = -1;
}


// select socket
bool	Socket::update()
{
#ifdef I_AM_SERVER
	return updateserver();
#else
	return updateclient();
#endif
}

bool	Socket::updateserver()
{
#ifdef I_AM_SERVER
	if (clisock == -1)
	{
		ZeroMemory(&cli_addr, sizeof(struct sockaddr_in));
		int sockLen = sizeof(struct sockaddr_in);
		clisock = accept(sock, (struct sockaddr*)&cli_addr, &sockLen);
		if (clisock == INVALID_SOCKET)
		{
			Logger::log(LOG_INFO, "Accept Error\n");
#if WIN32
			::closesocket(sock);
#else
			close(sock);
#endif
			return false;
		}

		// client socket --> nonblock
		unsigned long arg = 1;
		if (ioctlsocket(clisock, FIONBIO, &arg) != 0) return false;
		Logger::log(LOG_INFO, "New Client Come !!\n");
	}
	else
	{
		fd_set read_flags, write_flags;
		struct timeval waitd;          // the max wait time for an event
		int sel;

		waitd.tv_sec = 0;
		waitd.tv_usec = 1000;		// micro second
		FD_ZERO(&read_flags);
		FD_ZERO(&write_flags);
		FD_SET(clisock, &read_flags);

		sel = select(clisock + 1, &read_flags, &write_flags, (fd_set*)0, &waitd);
		if (sel < 0) return true;	// 아무것도 없다!

		// 읽을것이 있으면 read
		if (FD_ISSET(clisock, &read_flags))
		{
			FD_CLR(clisock, &read_flags);
			char in[SOCKET_BUFFER];
			memset(&in, 0, sizeof(in));

			int recvsize = ::recv(clisock, in, sizeof(in), 0);
			if( recvsize > 0)
			{
				if (recvbuffer.totalsize > 0)
				{
					// 뒤에 이어 받아야함
					memcpy(recvbuffer.buffer + recvbuffer.totalsize, in, recvsize);
					recvbuffer.totalsize += recvsize;
					recvdone();
				}
				else
				{
					// 처음 받음
					recvbuffer.totalsize = recvsize;
					memcpy(recvbuffer.buffer, in, recvsize);
					recvdone();
				}
			}
			else
			{
#if WIN32
				::closesocket(clisock);
#else
				close(clisock);
#endif
				Logger::log(LOG_INFO, "Client Disconnected !\n");
				clisock = -1;
				return true;
			}

		}

		// 보낼것이 있으면 보낸다로 설정
		if (sendbuffer.totalsize > 0)
			FD_SET(clisock, &write_flags);

		// 보냄
		if (FD_ISSET(clisock, &write_flags))
		{
			FD_CLR(clisock, &write_flags);
			int sendsize = ::send(clisock, sendbuffer.buffer + sendbuffer.currentsize, sendbuffer.totalsize - sendbuffer.currentsize, 0);
			if (sendbuffer.totalsize == sendbuffer.currentsize + sendsize)
			{
				senddone();
			}
			else
			{
				sendbuffer.currentsize += sendsize;
			}
		}
	}
#endif
	return true;
}

bool	Socket::updateclient()
{
#if 1
	int sel;

	char in[SOCKET_BUFFER];
	memset(&in, 0, sizeof(in));

	int recvsize = ::recv(sock, in, sizeof(in), 0);
	if (recvsize <= 0)
	{
		Logger::log(CLIENT_LOG_ERR, "Socket recv. Error!\n");
		closesocket();
		return false;
	}
	else
	{
		if (recvbuffer.totalsize > 0)
		{
			// 뒤에 이어 받아야함
			memcpy(recvbuffer.buffer + recvbuffer.totalsize, in, recvsize);
			recvbuffer.totalsize += recvsize;
			recvdone();
		}
		else
		{
			// 처음 받음
			recvbuffer.totalsize = recvsize;
			memcpy(recvbuffer.buffer, in, recvsize);
			recvdone();
		}
	}

/*
	// 보낼것이 있으면 보낸다로 설정
	if (sendbuffer.totalsize > 0)
		FD_SET(sock, &write_flags);

	// 보냄
	if (FD_ISSET(sock, &write_flags))
	{
		FD_CLR(sock, &write_flags);
		int sendsize = ::send(sock, sendbuffer.buffer + sendbuffer.currentsize, sendbuffer.totalsize - sendbuffer.currentsize, 0);
		if (sendbuffer.totalsize == sendbuffer.currentsize + sendsize)
		{
			senddone();
		}
		else
		{
			sendbuffer.currentsize += sendsize;
		}
	}
*/
#else
	fd_set read_flags, write_flags;
	struct timeval waitd;          // the max wait time for an event
	int sel;

	waitd.tv_sec = 0;
	waitd.tv_usec = 0;		// micro second
	FD_ZERO(&read_flags);
	FD_ZERO(&write_flags);
	FD_SET(sock, &read_flags);

	sel = select(sock + 1, &read_flags, &write_flags, (fd_set*)0, &waitd);
	if (sel < 0) return true;	// 아무것도 없다!

								// 읽을것이 있으면 read
	if (FD_ISSET(sock, &read_flags))
	{
		FD_CLR(sock, &read_flags);

		char in[SOCKET_BUFFER];
		memset(&in, 0, sizeof(in));

		int recvsize = ::recv(sock, in, sizeof(in), 0);
		if (recvsize <= 0)
		{
			Logger::log(CLIENT_LOG_ERR, "Socket recv. Error!\n");
			closesocket(); 
			return false;
		}
		else
		{
			if (recvbuffer.totalsize > 0)
			{
				// 뒤에 이어 받아야함
				memcpy(recvbuffer.buffer + recvbuffer.totalsize, in, recvsize);
				recvbuffer.totalsize += recvsize;
				recvdone();
			}
			else
			{
				// 처음 받음
				recvbuffer.totalsize = recvsize;
				memcpy(recvbuffer.buffer, in, recvsize);
				recvdone();
			}
		}
	}

	// 보낼것이 있으면 보낸다로 설정
	if (sendbuffer.totalsize > 0)
		FD_SET(sock, &write_flags);

	// 보냄
	if (FD_ISSET(sock, &write_flags))
	{
		FD_CLR(sock, &write_flags);
		int sendsize = ::send(sock, sendbuffer.buffer + sendbuffer.currentsize, sendbuffer.totalsize - sendbuffer.currentsize, 0);
		if (sendbuffer.totalsize == sendbuffer.currentsize + sendsize)
		{
			senddone();
		}
		else
		{
			sendbuffer.currentsize += sendsize;
		}
	}
#endif
	return true;
}


void	Socket::recvdone()
{
	while (1)
	{
		if (recvbuffer.totalsize >= sizeof(int) + sizeof(char))	// data size + packet
		{
			int datasize = (int&)*recvbuffer.buffer;
			if (recvbuffer.totalsize >= sizeof(int) + sizeof(char) + datasize)
			{
				SocketBuffer buffer;
				buffer.totalsize = sizeof(int) + sizeof(char) + datasize;
				memcpy(buffer.buffer, recvbuffer.buffer, buffer.totalsize);
				recvbufferlist.push_back(buffer);

				recvbuffer.totalsize -= buffer.totalsize;

				// 남아있는게 있는가?
				if (recvbuffer.totalsize > 0)
				{
					char tempbuffer[SOCKET_BUFFER] = { 0, };
					memcpy(tempbuffer, recvbuffer.buffer + buffer.totalsize, recvbuffer.totalsize);
					memcpy(recvbuffer.buffer, tempbuffer, SOCKET_BUFFER);
				}
			}
			else
				break;
		}
		else
			break;
	}
}

void	Socket::senddone()
{
	if (!sendbufferlist.empty())
	{
		sendbuffer.totalsize = sendbufferlist[0].totalsize;
		sendbuffer.currentsize = 0;
		memcpy(sendbuffer.buffer, sendbufferlist[0].buffer, sendbufferlist[0].totalsize);
		sendbufferlist.pop_front();
	}
	else
	{
		sendbuffer.totalsize = -1;
		sendbuffer.currentsize = 0;
		memset(sendbuffer.buffer, 0, SOCKET_BUFFER);
	}
}

bool	Socket::sendpacket(char *packet,int packetsize)
{
	if (sendbuffer.totalsize > 0)
	{
		// 전송 중이라 쌓아 놓는다
		SocketBuffer buf;
		buf.totalsize = packetsize;
		memcpy(buf.buffer, packet, packetsize);
		sendbufferlist.push_back(buf);
	}
	else
	{
		// 바로 전송할거로 이동
		sendbuffer.totalsize = packetsize;
		sendbuffer.currentsize = 0;
		memcpy(sendbuffer.buffer, packet, packetsize);
	}
	return true;
}

// Read packet for parse
bool	Socket::recvpacket(SocketBuffer *buffer)
{
	if( !recvbufferlist.empty() )
	{
		buffer->totalsize = recvbufferlist[0].totalsize;
		buffer->currentsize = recvbufferlist[0].currentsize;
		memcpy(buffer->buffer, recvbufferlist[0].buffer, SOCKET_BUFFER);
		recvbufferlist.pop_front();
		return true;
	}

	return false;
}


int		Socket::sendImmediate(char *buf, int datasize)
{
	return ::send(sock, buf, datasize, 0);
}