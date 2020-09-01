#pragma once

#include <gphoto2/gphoto2-camera.h>

class cameracontext
{
public :

	//static cameracontext* instance;
	static cameracontext* getInstance()
	{
		if (_instance == nullptr)
			_instance = new cameracontext();
		return _instance;
	};

	GPContext* getcontext();

private :
	cameracontext();
	~cameracontext();

	static cameracontext* _instance;

};
