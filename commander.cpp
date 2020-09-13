
#include "commander.h"

Commander::Commander()
{
}

Commander::~Commander()
{
}

void Commander::addcommand(char packet, char *data, int datalen)
{
	Command com;
	com.packet = packet;
	//memcpy(com.data, data, datalen);

	commandlist.push_back(com);
}
