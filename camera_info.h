#pragma once

#include <string>

#define CAMERA_INFO_ISO				1000	// 감도
#define CAMERA_INFO_SHUTTERSPEED	1100	// 셔터 스피드
#define CAMERA_INFO_APERTURE		1200	// 조리개 (F1.4, F2, F2.8) 값이 작을수록 빛을 많이 받음


class Camera_Info
{
public :
	Camera_Info() {};
	~Camera_Info() {};

	std::vector<std::string>	iso;
	std::vector<std::string>	shutterspeed;
	std::vector<std::string>	aperture;

};
