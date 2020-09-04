#pragma once

#include <string>

#define CAMERA_INFO_ISO				1000	// ����
#define CAMERA_INFO_SHUTTERSPEED	1100	// ���� ���ǵ�
#define CAMERA_INFO_APERTURE		1200	// ������ (F1.4, F2, F2.8) ���� �������� ���� ���� ����


class Camera_Info
{
public :
	Camera_Info() {};
	~Camera_Info() {};

	std::vector<std::string>	iso;
	std::vector<std::string>	shutterspeed;
	std::vector<std::string>	aperture;

};
