#if 0

#pragma once

#include <vector>



class cam_controller
{
public :
	cam_controller();
	~cam_controller();

	bool init();
	bool setcam_param();
	
	bool take_shot();


private:

	void detectAllCam();

	//std::vector<gphoto2pp::CameraWrapper>	cameralist;


};

#endif