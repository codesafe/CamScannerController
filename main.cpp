#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#include <gphoto2/gphoto2.h>
#include <gphoto2/gphoto2-camera.h>

#include "./samples.h"
#include "./camera_context.h"
#include "./camera_action.h"
#include "./utils.h"

int main(int argc, char** argv) 
{
	int	retval;
	GPContext* context = Camera_Context::getInstance()->getcontext();
	Camera* camera = Camera_Context::getInstance()->getcamera();

	FILE* f;
	char* data;
	unsigned long size;

	//gp_log_add_func(GP_LOG_ERROR, errordumper, NULL);

	retval = gp_camera_init(camera, context);
	if (retval != GP_OK) 
	{
		printf("  Retval of capture_to_file: %d\n", retval);
		exit(1);
	}

	GPParams param;
	param.camera = camera;
	param.context = context;

	Camera_Action* action = new Camera_Action();
	action->get_config(&param, "iso");
 	action->get_config(&param, "shutterspeed");
// 	action->get_camera_config(&param, "aeb");
 	action->get_config(&param, "aperture");

	//int r = action->get_camera_config(&param, "iso");

	printf("Set Config\n");
	action->set_camera_config(&param, "iso", 0);
	action->set_camera_config(&param, "shutterspeed", 36);
	action->set_camera_config(&param, "aperture", 5);

	// ¹Ý¼ÅÅÍ
	printf("Half Press\n");
	action->set_camera_config(&param, "eosremoterelease", 6);
	action->action_camera_wait_focus(&param);

	//action->action_camera_wait_event(&param, "3s");

	printf("Shot\n");
	action->shot_to_file(&param, "test2.jpg");
	//action->set_camera_config(&param, "eosremoterelease", 5);

	gp_camera_exit(camera, context);
	return 0;
}

