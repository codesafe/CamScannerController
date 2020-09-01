#if 0

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <gphoto2/gphoto2-camera.h>

#include "samples.h"

/* Sample autodetection program.
 *
 * This program can autodetect a single camera and then calls a
 * simple function in it (summary).
 */

int main(int argc, char** argv) {
	Camera* camera;
	int		ret;
	char* owner;
	GPContext* context;
	CameraText	text;

	context = sample_create_context(); /* see context.c */
	gp_camera_new(&camera);

	/* This call will autodetect cameras, take the
	 * first one from the list and use it. It will ignore
	 * any others... See the *multi* examples on how to
	 * detect and use more than the first one.
	 */
	ret = gp_camera_init(camera, context);
	if (ret < GP_OK) {
		printf("No camera auto detected.\n");
		gp_camera_free(camera);
		return 0;
	}

	/* Simple query the camera summary text */
	ret = gp_camera_get_summary(camera, &text, context);
	if (ret < GP_OK) {
		printf("Camera failed retrieving summary.\n");
		gp_camera_free(camera);
		return 0;
	}
	printf("Summary:\n%s\n", text.text);

	/* Simple query of a string configuration variable. */
	ret = get_config_value_string(camera, "owner", &owner, context);
	if (ret >= GP_OK) {
		printf("Owner: %s\n", owner);
		free(owner);
	}
	gp_camera_exit(camera, context);
	gp_camera_free(camera);
	gp_context_unref(context);
	return 0;
}


#else

#include "./include/gphoto2pp/helper_gphoto2.hpp"
#include "./include/gphoto2pp/camera_wrapper.hpp"
#include "./include/gphoto2pp/exceptions.hpp"

#include <string>


#include <iostream>

int main(int argc, char* argv[]) 
{
	std::string model;
	std::string port;

	// runs the autodetect method and returns the first camera we find (if any)
	std::cout << "#############################" << std::endl;
	std::cout << "# autoDetect - first camera #" << std::endl;
	std::cout << "#############################" << std::endl;
	try
	{
		auto cameraPair = gphoto2pp::autoDetect();
		std::cout << "model: " << cameraPair.first << " port: " << cameraPair.second << std::endl;

		model = cameraPair.first;
		port = cameraPair.second;
	}
	catch (gphoto2pp::exceptions::NoCameraFoundError & e)
	{
		std::cout << "gphoto2 couldn't detect any cameras connected to the computer" << std::endl;
		std::cout << "Exception Message: " << e.what() << std::endl;
	}

	try
	{
		std::cout << "connecting to the first camera (model and port above)..." << std::endl;
		auto cameraWrapper = gphoto2pp::CameraWrapper(model, port);

		std::cout << "#############################" << std::endl;
		std::cout << "# print camera summary      #" << std::endl;
		std::cout << "#############################" << std::endl;
		std::cout << cameraWrapper.getSummary() << std::endl;
	}
	catch (gphoto2pp::exceptions::gphoto2_exception & e)
	{
		std::cout << "gphoto2 Exception Code: " << e.getResultCode() << std::endl;
		std::cout << "Exception Message: " << e.what() << std::endl;
	}
}

#endif