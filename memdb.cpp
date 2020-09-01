#include <stdio.h> 
#include "memdb.h"

MemDB *	MemDB::instance = NULL;

MemDB::MemDB()
{
}

MemDB::~MemDB()
{
}

void	MemDB::reset()
{
	keyvalue.clear();
}

bool	MemDB::getBoolValue(std::string key, bool d)
{
	std::string ret = getvalue(key);
	if (ret.empty())
		return d;

	return ret == "true" ? true : false;
}

int		MemDB::getIntValue(std::string key, int d)
{
	std::string ret = getvalue(key);

	if( ret.empty() )
		return d;

	return atoi(ret.c_str());
}

float	MemDB::getFloatValue(std::string key, float d)
{
	std::string ret = getvalue(key);

	if( ret.empty() )
		return d;

	return (float)atof(ret.c_str());
}

std::string		MemDB::getValue(std::string key)
{
	return getvalue(key);
}

void	MemDB::setValue(std::string key, int value)
{
	char valuestr[256];
#ifdef WIN32
	_snprintf (valuestr, sizeof (valuestr), "%d", value);
#else
	snprintf (valuestr, sizeof (valuestr), "%d", value);
#endif

	setvalue(key, valuestr);
}

void	MemDB::setValue(std::string key, float value)
{
	char valuestr[256];
#ifdef WIN32
	_snprintf (valuestr, sizeof (valuestr), "%f", value);
#else
	snprintf (valuestr, sizeof (valuestr), "%f", value);
#endif

	setvalue(key, valuestr);
}

void	MemDB::setValue(std::string key, std::string value)
{
	setvalue(key, value);
}

std::string MemDB::getvalue(std::string key)
{
	std::map<std::string, std::string>::iterator it = keyvalue.find(key);
	if( it != keyvalue.end() )
	{
		return it->second;
	}

	return std::string("");
}

void MemDB::setvalue(std::string key, std::string value)
{
	// 없으면 생성 , 있으면 갱신
	std::map<std::string, std::string>::iterator it = keyvalue.find(key);
	if( it == keyvalue.end() )
	{
		// 없어서 추가
		keyvalue.insert(std::make_pair(key, value));
		return;
	}

	it->second = value;
}