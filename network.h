#ifndef _NETWORK_
#define _NETWORK_

#include "socket.h"

class Commander;

class Network
{
public :
	Network();
	~Network();

	void	init();
	void	uninit();
	bool	connect(int camnum);

	void	update();
	bool	read();
	bool	write(char packet, char *data, int datasize);
	bool	getenable() { return enable;  }

	Commander *getcommander() { return commander; }

private :
	void	parsepacket(SocketBuffer *buffer);
	//int		getpackettype(char packet);

private :
	bool		enable;
	Commander	*commander;
	Socket		*socket;

};


#endif