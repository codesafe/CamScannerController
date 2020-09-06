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

#include "./predef.h"
#include "./camera_context.h"
#include "./camera_action.h"
#include "./utils.h"
#include "./network.h"


void mainupdate()
{
	while (true)
	{
		Utils::Sleep(10);

		Network::getinstance()->update();

// 		int ret = Commander::getinstance()->update();
// 		if (ret != 0)
// 		{
// 			if (ret == DEVICERESET)
// 			{
// 				restartapp();
// 				break;
// 			}
// 			else if (ret == FORCEPATCH)
// 			{
// 
// 			}
// 		}
	}
}




int main(int argc, char** argv) 
{
	int	retval;
	//GPPortInfoList* portinfo_list = NULL;

	//_get_portinfo_list(portinfo_list);


	CameraManager::getInstance()->GetCameraList();
	CameraManager::getInstance()->CreateAllCamera();


/*
	char* port = "usb:001,011";
	char verified_port[1024];
	verified_port[sizeof(verified_port) - 1] = '\0';
	if (!strchr(port, ':')) 
	{
		if (!strcmp(port, "usb")) 
		{
			strncpy(verified_port, "usb:",sizeof(verified_port) - 1);
		}
		else if (strncmp(port, "/dev/", 5) == 0) {
			strncpy(verified_port, "serial:", sizeof(verified_port) - 1);
			strncat(verified_port, port, sizeof(verified_port) - strlen(verified_port) - 1);
		}
		else if (strncmp(port, "/proc/", 6) == 0) 
		{
			strncpy(verified_port, "usb:", sizeof(verified_port) - 1);
			strncat(verified_port, port, sizeof(verified_port) - strlen(verified_port) - 1);
		}
	}
	else
		strncpy(verified_port, port, sizeof(verified_port) - 1);
*/

/*
	char verified_port[1024] = { "usb:001,010\0" };

	GPPortInfo portinfo;
	int p = gp_port_info_list_lookup_path(param.portinfo_list, verified_port);
	int r = gp_port_info_list_get_info(param.portinfo_list, p, &portinfo);
	Camera_Context::getInstance()->setcamera_port(camera, portinfo);


	Camera_Action* action = new Camera_Action();
	action->shot_to_file(&param, "test2.jpg");
	gp_camera_exit(camera, context);
*/

	return 0;

/*

	GPContext* context = Camera_Context::getInstance()->getcontext();
	Camera* camera = Camera_Context::getInstance()->getcamera();

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
	param.portinfo_list = NULL;

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
*/
}

