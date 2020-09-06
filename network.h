#ifndef _NETWORK_
#define _NETWORK_

#include "socket.h"

class Network
{
public :
	static Network * getinstance()
	{
		if (instance == NULL)
			instance = new Network();
		return instance;
	}

	void	init();
	void	uninit();
	bool	connect();

	void	update();
	bool	read();
	bool	write(char packet, char *data, int datasize);
	bool	getenable() { return enable;  }

private :
	void	parsepacket(SocketBuffer *buffer);
	int		getpackettype(char packet);

private :
	Network();
	~Network();

	static Network *	instance;

	bool		enable;
	Socket	*	socket;
};


#endif