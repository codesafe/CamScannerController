#pragma once

#include <gphoto2/gphoto2-camera.h>

class cameraaction
{
public:
	cameraaction();
	~cameraaction();

	void shot_to_file(Camera* camera, GPContext* context, char* fn);
};
