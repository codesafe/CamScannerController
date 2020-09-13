#if 0

#pragma once

#include "predef.h"


class Camera_Context
{
public :
	Camera_Context();
	~Camera_Context();

	GPParams* getParam() { return param; }

	void createCameraContext();
	void destroyCameraContext();
	int setCameraPort(GPPortInfo &info);

	void setParam(CAMERA_PARAM param, int value);

private:
	GPParams* param;

	int iso;
	int shutterspeed;
	int aperture;

	static GPContextErrorFunc _error_callback(GPContext* context, const char* text, void* data);
	static GPContextMessageFunc _message_callback(GPContext* context, const char* text, void* data);

};

//////////////////////////////////////////////////////////////////////////

class CameraManager
{
public:
	static CameraManager* getInstance()
	{
		if (_instance == nullptr)
			_instance = new CameraManager();
		return _instance;
	};

	void GetCameraList();
	bool CreateAllCamera();
	int GetAllCameraLength();
	Camera_Context* GetCameraContext(int index);

	void DestroyCamera(int index);

private:
	static CameraManager* _instance;

	CameraManager();
	~CameraManager();

	int get_port_list(GPParams* gp_params);
	void _get_portinfo_list(GPParams* p);

private :

	std::vector<std::string>		detectedCameraNameList;
	std::vector<Camera_Context*>	cameraList;

	CameraAbilitiesList* gp_params_abilities_list(GPParams* p);


};

#endif