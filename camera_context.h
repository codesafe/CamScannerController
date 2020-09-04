#pragma once

#include <gphoto2/gphoto2-camera.h>

class Camera_Context
{
public :
	static Camera_Context* getInstance()
	{
		if (_instance == nullptr)
			_instance = new Camera_Context();
		return _instance;
	};

	GPContext* getcontext();
	Camera* getcamera();

private :
	static Camera_Context* _instance;

	Camera_Context();
	~Camera_Context();

private:
	Camera* camera;
	GPContext* context;

};
