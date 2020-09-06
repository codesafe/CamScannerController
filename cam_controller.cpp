#if 0

#include <string>
#include <iostream>

#include "cam_controller.h"

// #include "./include/gphoto2pp/helper_gphoto2.hpp"
// #include "./include/gphoto2pp/camera_wrapper.hpp"
// #include "./include/gphoto2pp/exceptions.hpp"

#include <gphoto2pp/helper_camera_wrapper.hpp>
#include <gphoto2pp/camera_wrapper.hpp>
#include <gphoto2pp/camera_file_wrapper.hpp>
#include <gphoto2pp/camera_capture_type_wrapper.hpp>
#include <gphoto2pp/exceptions.hpp>

#include <gphoto2pp/window_widget.hpp>
#include <gphoto2pp/toggle_widget.hpp>

#include <gphoto2pp/helper_gphoto2.hpp>
#include <gphoto2pp/camera_list_wrapper.hpp>




cam_controller::cam_controller()
{

}

cam_controller::~cam_controller()
{
	
}


bool cam_controller::init()
{
	detectAllCam();



	return true;
}

bool cam_controller::setcam_param()
{




	return true;
}

bool cam_controller::take_shot()
{

	std::cout << "#############################" << std::endl;
	std::cout << "# connect to camera         #" << std::endl;
	std::cout << "#############################" << std::endl;
	gphoto2pp::CameraWrapper cameraWrapper; // Not passing in model and port will connect to the first available camera.


	bool capture = true;
	/*setCanonCapture(cameraWrapper, true);*/
	try
	{
		auto captureWidget = cameraWrapper.getConfig().getChildByName<gphoto2pp::ToggleWidget>("capture");

		captureWidget.setValue(capture ? 1 : 0);
		cameraWrapper.setConfig(captureWidget);
	}
	catch (const std::runtime_error & e)
	{
		// Swallow the exception
		std::cout << "Tried to set canon capture, failed. The camera is probably not a canon" << std::endl;
	}

	std::cout << "#############################" << std::endl;
	std::cout << "# Capture Method 1          #" << std::endl;
	std::cout << "#############################" << std::endl;
	std::cout << "Taking first picture..." << std::endl << std::endl;
	// Clean and quick capture and save to disk, but this assumes you are taking images, and in jpeg foramt. Adjust type and extension as appropriate.
	gphoto2pp::helper::capture(cameraWrapper, "example4_capture_method_2.jpg", true);


	return true;
}


void cam_controller::detectAllCam()
{
//	cameralist.clear();

	auto cameraList = gphoto2pp::autoDetectAll();

	for (int i = 0; i < cameraList.count(); ++i)
	{
		std::cout << "model: " << cameraList.getName(i) << std::endl;
	}
}

#endif