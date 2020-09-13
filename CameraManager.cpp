#include "CameraManager.h"
#include "CameraController.h"

CameraMan* CameraMan::_instance = NULL;

CameraMan::CameraMan()
{
	_getCameralist = false;
}

CameraMan::~CameraMan()
{
}

void CameraMan::enumCameraList()
{
	if (_getCameralist == false)
	{
		cameraIDlist.clear();

		//////////////////////////////////////////////////////////////////////////

		Camera* camera = NULL;
		GPContext* context = NULL;
		GPPortInfoList* portinfo_list = NULL;
		CameraAbilitiesList* _abilities_list = NULL;


		gp_camera_new(&camera);
		context = gp_context_new();

		//gp_context_set_error_func(context, (GPContextErrorFunc)Camera_Context::_error_callback, NULL);
		//gp_context_set_status_func(context, (GPContextMessageFunc)Camera_Context::_message_callback, NULL);

		int ret = gp_camera_init(camera, context);
		if (ret < GP_OK)
			gp_camera_free(camera);

		//////////////////////////////////////////////////////////////////////////

		int count, result;
		if (gp_port_info_list_new(&portinfo_list) < GP_OK)
			return;

		result = gp_port_info_list_load(portinfo_list);
		if (result < 0)
		{
			gp_port_info_list_free(portinfo_list);
			return;
		}

		count = gp_port_info_list_count(portinfo_list);
		if (count < 0)
		{
			gp_port_info_list_free(portinfo_list);
			return;
		}

		GPPortInfo info;
		for (int x = 0; x < count; x++)
		{
			char* xname, * xpath;
			result = gp_port_info_list_get_info(portinfo_list, x, &info);
			if (result < 0)
				break;
			gp_port_info_get_name(info, &xname);
			gp_port_info_get_path(info, &xpath);
			printf("%-32s %-32s\n", xpath, xname);
		}

		//////////////////////////////////////////////////////////////////////////

		{
			const char* name = NULL, * value = NULL;
			CameraList* list;
			gp_list_new(&list);
			gp_abilities_list_detect(gp_params_abilities_list(context, _abilities_list), portinfo_list, list, context);

			int count = gp_list_count(list);

			printf("%-30s %-16s\n", "Model", "Port");
			printf("----------------------------------------------------------\n");
			for (int x = 0; x < count; x++)
			{
				gp_list_get_name(list, x, &name);
				gp_list_get_value(list, x, &value);
				printf("%-30s %-16s\n", name, value);

				cameraIDlist.push_back(std::string(value));
			}
			gp_list_free(list);

		}

		if(camera != NULL)
			gp_camera_exit(camera, context);
		if( context != NULL )
			gp_context_unref(context);

		if (portinfo_list != NULL)
			gp_port_info_list_free(portinfo_list);
		if (portinfo_list != NULL)
			gp_abilities_list_free(_abilities_list);

		_getCameralist = true;
	}

	// create all cameras
	if (_getCameralist)
	{
		for (int i=0; i< cameraIDlist.size(); i++)
		{
			CameraController* camera = new CameraController();
			camera->init();
			camera->setPort(cameraIDlist[i]);
			cameraList.push_back(camera);
		}
	}


}

CameraAbilitiesList* CameraMan::gp_params_abilities_list(GPContext* context, CameraAbilitiesList* _abilities_list)
{
	/* If p == NULL, the behaviour of this function is as undefined as
	 * the expression p->abilities_list would have been. */
	if (_abilities_list == NULL)
	{
		gp_abilities_list_new(&_abilities_list);
		gp_abilities_list_load(_abilities_list, context);
	}
	return _abilities_list;
}

// °¹¼ö
int CameraMan::getEnumCameraNum()
{
	return cameraIDlist.size();
}

CameraController* CameraMan::getCamera(int i)
{
	if (cameraList.size() <= i)
		return NULL;

	return cameraList[i];
}