#pragma once

#include "predef.h"

class CameraController;

class CameraMan
{
public:
	static CameraMan* getInstance()
	{
		if (_instance == NULL)
			_instance = new CameraMan();
		return _instance;
	}

	void enumCameraList();
	int getEnumCameraNum();		// °¹¼ö
	CameraController* getCamera(int i);

private:
	CameraMan();
	~CameraMan();

	CameraAbilitiesList* gp_params_abilities_list(GPContext* context, CameraAbilitiesList* _abilities_list);


private:
	static CameraMan* _instance;

	bool _getCameralist;

	std::vector<string>				cameraIDlist;
	std::vector<CameraController*>	cameraList;

};