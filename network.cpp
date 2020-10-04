#include "network.h"
#include "commander.h"

Network::Network()
{
	enable = false;
	socket = NULL;
	commander = NULL;
}

Network::~Network()
{

}

void	Network::init()
{
	socket = new Socket();
	socket->init();

	commander = new Commander();
}

void	Network::uninit()
{
	socket->uninit();
	delete socket;
	socket = NULL;

	delete commander;

	enable = false;
}

bool	Network::connect(int camnum)
{
	enable = socket->connect();
	Logger::log(CLIENT_LOG_INFO, "socket connect : %d :%s", camnum, enable ? "success" : "fail");

	return enable;
}

void	Network::update()
{
	if( enable )
	{
		bool ret = socket->update();
		if( ret == false)
		{
			Logger::log(CLIENT_LOG_ERR, "socket disconnected");
			enable = false;
		}

		read();
	}
}

// Parse recieved packet
bool	Network::read()
{
	// recieve and parse
	if( enable )
	{
		SocketBuffer buffer;
		bool ret = socket->recvpacket(&buffer);
		if( ret == true )
		{
			// packet parse
			parsepacket(&buffer);
		}
	}
	else
		return false;

	return true;
}

// send packet to server
bool	Network::write(char packet, char *data, int datasize)
{
	if(enable)
	{
		char buff[SOCKET_BUFFER] = { 0, };
		memcpy(buff, &datasize, sizeof(int));	// size header는 패킷 데이터 크기

		buff[4] = packet;
		memcpy(buff+sizeof(int)+sizeof(char), data, datasize);
		return socket->sendpacket(buff, datasize + sizeof(int) + sizeof(char));
	}

	return false;
}

void	Network::parsepacket(SocketBuffer *buffer)
{
	int datasize = (int&)*(buffer->buffer);
	char packet = (char&)*(buffer->buffer+sizeof(int));

	commander->addcommand(packet, buffer->buffer + sizeof(int) + sizeof(char), datasize);
}


int		Network::sendImmediate(char* data, int datasize)
{
	return socket->sendImmediate(data, datasize);
}
