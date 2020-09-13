#ifndef _COMMANDER_
#define _COMMANDER_

#include "predef.h"
#include <vector>

struct Command
{
	//int	type;
	char packet;
	char data[SOCKET_BUFFER];

	Command()
	{
		packet = 0;
		memset(data, 0, SOCKET_BUFFER);
	}
};

class Commander
{
public :
	Commander();
	~Commander();

	void	addcommand(char packet, char *data, int datalen);
	std::vector<Command>	commandlist;
};



#endif