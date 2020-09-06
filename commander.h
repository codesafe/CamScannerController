#ifndef _COMMANDER_
#define _COMMANDER_

#include "predef.h"
#include <deque>

struct Command
{
	int	type;
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
	static Commander * getinstance()
	{
		if (instance == NULL)
			instance = new Commander();
		return instance;
	}

	void	addcommand(int type, char packet, char *data, int datalen);
	int		update();

private :	
	Commander();
	~Commander();
	
	static Commander*	instance;
	std::deque<Command>	commandlist;
};



#endif