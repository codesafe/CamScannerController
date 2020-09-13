#if 0

#include <stdio.h>
#include <gphoto2/gphoto2-camera.h>
#include <gphoto2/gphoto2-list.h>
#include <gphoto2/gphoto2-port-log.h>
#include <gphoto2/gphoto2-setting.h>
#include <gphoto2/gphoto2-filesys.h>
//#include <gphoto2/globals.h>

#include "camera_context.h"
#include "logger.h"
#include "camera_action.h"

//////////////////////////////////////////////////////////////////////////


Camera_Context::Camera_Context()
{
	param = NULL;
}

Camera_Context::~Camera_Context()
{
	gp_camera_exit(param->camera, param->context);
	gp_context_unref(param->context);

	if(param->portinfo_list != NULL)
		gp_port_info_list_free(param->portinfo_list);

	if (param->_abilities_list != NULL)
		gp_abilities_list_free(param->_abilities_list);

	delete param;
}

int Camera_Context::setCameraPort(GPPortInfo &info)
{
	return gp_camera_set_port_info(param->camera, info);
}

void Camera_Context::setParam(CAMERA_PARAM param, int value)
{
	switch (param)
	{
		case ISO:
			iso = value;
			break;
		case SHUTTERSPEED:
			shutterspeed = value;
			break;
		case APERTURE:
			aperture = value;
			break;
	}
}

void Camera_Context::createCameraContext()
{
	// 기본값 설정
	iso = ISO_VALUE;
	shutterspeed = SHUTTERSPEED_VALUE;
	aperture = APERTURE_VALUE;

	param = new GPParams();

	gp_camera_new(&param->camera);
	param->context = gp_context_new();
	gp_context_set_error_func(param->context, (GPContextErrorFunc)Camera_Context::_error_callback, NULL);
	gp_context_set_status_func(param->context, (GPContextMessageFunc)Camera_Context::_message_callback, NULL);

	int ret = gp_camera_init(param->camera, param->context);
	if (ret < GP_OK)
		gp_camera_free(param->camera);
}

void Camera_Context::destroyCameraContext()
{
}

GPContextErrorFunc Camera_Context::_error_callback(GPContext* context, const char* text, void* data)
{
	return 0;
}

GPContextMessageFunc Camera_Context::_message_callback(GPContext* context, const char* text, void* data)
{
	return 0;
}


//////////////////////////////////////////////////////////////////////////

CameraManager* CameraManager::_instance = NULL;

CameraManager::CameraManager()
{
}

CameraManager::~CameraManager()
{
}

void CameraManager::GetCameraList()
{
	Camera_Context* context = new Camera_Context();
	context->createCameraContext();
	GPParams* p = context->getParam();

	int count, result;
	if (gp_port_info_list_new(&p->portinfo_list) < GP_OK)
		return;

	result = gp_port_info_list_load(p->portinfo_list);
	if (result < 0)
	{
		gp_port_info_list_free(p->portinfo_list);
		return;
	}

	count = gp_port_info_list_count(p->portinfo_list);
	if (count < 0)
	{
		gp_port_info_list_free(p->portinfo_list);
		return;
	}

	GPPortInfo info;
	for (int x = 0; x < count; x++)
	{
		char* xname, * xpath;
		result = gp_port_info_list_get_info(p->portinfo_list, x, &info);
		if (result < 0)
			break;
		gp_port_info_get_name(info, &xname);
		gp_port_info_get_path(info, &xpath);
		printf("%-32s %-32s\n", xpath, xname);
	}

	//////////////////////////////////////////////////////////////////////////

	{
		detectedCameraNameList.clear();

		const char* name = NULL, * value = NULL;
		CameraList* list;
		gp_list_new(&list);
		gp_abilities_list_detect(gp_params_abilities_list(p), p->portinfo_list, list, p->context);

		int count = gp_list_count(list);

		printf("%-30s %-16s\n", "Model", "Port");
		printf("----------------------------------------------------------\n");
		for (int x = 0; x < count; x++) 
		{
			gp_list_get_name(list, x, &name);
			gp_list_get_value(list, x, &value);
			printf("%-30s %-16s\n", name, value);

			detectedCameraNameList.push_back(std::string(value));
		}
		gp_list_free(list);

	}

	delete context;
}


CameraAbilitiesList* CameraManager::gp_params_abilities_list(GPParams* p)
{
	/* If p == NULL, the behaviour of this function is as undefined as
	 * the expression p->abilities_list would have been. */
	if (p->_abilities_list == NULL) 
	{
		gp_abilities_list_new(&p->_abilities_list);
		gp_abilities_list_load(p->_abilities_list, p->context);
	}
	return p->_abilities_list;
}


bool CameraManager::CreateAllCamera()
{
	for (int i = 0; i < detectedCameraNameList.size(); i++)
	{
		Camera_Context* context = new Camera_Context();
		context->createCameraContext();
		GPParams* param = context->getParam();

		get_port_list(param);

		char verified_port[64] = { 0, };
		strcpy(verified_port, detectedCameraNameList[i].c_str());
		//char verified_port[64] = { "usb:001,006\0" };

		GPPortInfo portinfo;
		int p = gp_port_info_list_lookup_path(param->portinfo_list, verified_port);
 		if (p < GP_OK)
		{
			Logger::log(0, "Error gp_port_info_list_lookup_path : %d", p);
			return false;
		}

		int r = gp_port_info_list_get_info(param->portinfo_list, p, &portinfo);
		if (r < GP_OK)
		{
			Logger::log(0, "Error gp_port_info_list_get_info : %d", r);
			return false;
		}

		context->setCameraPort(portinfo);

		cameraList.push_back(context);
		Logger::log(0, "Create Camera : %s", verified_port);
		//printf("Create Camera : %s\n", verified_port);
	}

	return true;
}

int CameraManager::GetAllCameraLength()
{
	return cameraList.size();
}

Camera_Context* CameraManager::GetCameraContext(int index)
{
	if (index >= cameraList.size())
		return NULL;

	return cameraList[index];
}

void CameraManager::DestroyCamera(int index)
{
	if (index >= cameraList.size())
		return;

	delete cameraList[index];
}


int CameraManager::get_port_list(GPParams* gp_params)
{
	CameraList* list;
	CameraAbilities a;
	const char* model = NULL, * path = NULL;

	gp_camera_get_abilities(gp_params->camera, &a);
	_get_portinfo_list(gp_params);

	gp_list_new(&list);
	//gp_abilities_list_detect(gp_params_abilities_list(gp_params), gp_params->portinfo_list, list, gp_params->context));
	int count = gp_list_count(list);


	int use_auto = 1;
	if (count == 1 && !strcmp(a.model, ""))
	{
		/* Exactly one camera detected */
		gp_list_get_name(list, 0, &model);
		gp_list_get_value(list, 0, &path);
		if (a.model[0] && strcmp(a.model, model))
		{
			CameraAbilities alt;
			int m;

			//m = gp_abilities_list_lookup_model(gp_params_abilities_list(gp_params), model);
			//gp_abilities_list_get_abilities(gp_params_abilities_list(gp_params), m, &alt);

			if ((a.port == GP_PORT_USB) && (alt.port == GP_PORT_USB))
			{
				if ((a.usb_vendor == alt.usb_vendor) && (a.usb_product == alt.usb_product))
					use_auto = 0;
			}
		}
	}

	return GP_OK;
}

void CameraManager::_get_portinfo_list(GPParams* p)
{
	int count, result;
	GPPortInfoList* list = NULL;

	if (p->portinfo_list)
		return;

	if (gp_port_info_list_new(&list) < GP_OK)
		return;
	result = gp_port_info_list_load(list);
	if (result < 0) {
		gp_port_info_list_free(list);
		return;
	}
	count = gp_port_info_list_count(list);
	if (count < 0) {
		gp_port_info_list_free(list);
		return;
	}
	p->portinfo_list = list;
	return;
}

#endif