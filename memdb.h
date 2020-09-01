#ifndef _MEMDB_
#define _MEMDB_

#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <string>

class MemDB
{
public :
	static MemDB *	instance;
	static MemDB *getInstance()
	{
		if( instance == NULL )
			instance = new MemDB();			
		return instance;	
	};

	void			reset();

	bool			getBoolValue(std::string key, bool d = false);
	int				getIntValue(std::string key, int d = 0);
	float			getFloatValue(std::string key, float d = 0.f);
	std::string		getValue(std::string key);

	void			setValue(std::string key, int value);
	void			setValue(std::string key, float value);
	void			setValue(std::string key, std::string value);

private :

	std::string getvalue(std::string key);
	void setvalue(std::string key, std::string value);

	MemDB();
	~MemDB();

	std::map<std::string, std::string>	keyvalue;
};




#endif
