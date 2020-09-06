#pragma once

#include <string>
#include <gphoto2/gphoto2-camera.h>
#include <gphoto2/gphoto2-context.h>
#include <gphoto2/gphoto2-port-log.h>
#include <gphoto2/gphoto2-setting.h>
#include <gphoto2/gphoto2-filesys.h>
#include <gphoto2/gphoto2-abilities-list.h>


//struct GPParams;
#include "predef.h"


class Camera_Action
{
public:
	Camera_Action();
	~Camera_Action();

	int get_all_config(GPParams* p);
	int get_config(GPParams* p, char* name);

	int get_camera_config(GPParams* p, char* name);
	int set_camera_config(GPParams* p, char* name, int value);
	void shot_to_file(GPParams* p, char* fn);

	int action_camera_wait_event(GPParams* p, const char* arg);

	// half press
	int action_camera_wait_focus(GPParams* p);

	int get_port_list(GPParams* p);


private:
	int _find_widget_by_name(GPParams* p, const char* name, CameraWidget** child, CameraWidget** rootconfig);
	int print_widget(GPParams* p, const char* name, CameraWidget* widget);

	int get_widget(GPParams* p, const char* name, CameraWidget* widget);
	void _get_portinfo_list(GPParams* p);
	//CameraAbilitiesList* gp_params_abilities_list(GPParams* p);
};
