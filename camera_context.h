#pragma once

#include "predef.h"


class Camera_Context
{
public :
	Camera_Context();
	~Camera_Context();

	GPParams* getParam() { return param; }

	void createCameraContext();
	int setCameraPort(GPPortInfo &info);

private:
	GPParams* param;

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


private:
	static CameraManager* _instance;

	CameraManager();
	~CameraManager();

private :

	std::vector<std::string>		detectedCameraNameList;
	std::vector<Camera_Context*>	cameraList;

	CameraAbilitiesList* gp_params_abilities_list(GPParams* p);


};