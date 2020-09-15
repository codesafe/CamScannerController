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
#include <termio.h>
#include <vector>
#include <memory>

#include "predef.h"
#include "camera_context.h"
#include "camera_action.h"
#include "utils.h"
#include "network.h"
#include "commander.h"
#include "camera_thread.h"

#include "CameraController.h"
#include "CameraManager.h"


int getch(void) 
{
	int ch;
	struct termios buf, save;
	tcgetattr(0, &save);
	buf = save;
	buf.c_lflag &= ~(ICANON | ECHO);
	buf.c_cc[VMIN] = 1;
	buf.c_cc[VTIME] = 0;
	tcsetattr(0, TCSAFLUSH, &buf);
	ch = getchar();
	tcsetattr(0, TCSAFLUSH, &save);
	return ch;
}


int main(int argc, char** argv)
{

	int	retval;
	std::vector<CameraThread*> threadlist;
/*
	CameraMan::getInstance()->enumCameraList();

	int len = CameraMan::getInstance()->getEnumCameraNum();

	for (int i = 0; i < len; i++)
	{
		CameraThread* thread = new CameraThread();
		thread->Start(i);
		threadlist.push_back(thread);
	}
*/

	CameraThread* thread = new CameraThread();
	thread->Start(0);

	while (true)
	{
		Utils::Sleep(10);

		int i = getch();
		printf("Input = %d\n", i);

		if (i == '1')
		{
			for (int i = 0; i < threadlist.size(); i++)
			{
				threadlist[i]->addTestPacket(PACKET_HALFPRESS, i);
			}
		}
		else if (i == '2')
		{
			for (int i = 0; i < threadlist.size(); i++)
			{
				threadlist[i]->addTestPacket(PACKET_SHOT, i);
			}
		}
	}

	return 0;
}


int __main(int argc, char** argv)
{
	CameraMan::getInstance()->enumCameraList();

	int n = CameraMan::getInstance()->getEnumCameraNum();

	for (int i = 0; i < n; i++)
	{
		CameraController *cam = CameraMan::getInstance()->getCamera(i);
		cam->set_settings_value("eosremoterelease", "Press 1");
	}

	for (int i = 0; i < n; i++)
	{
		CameraController* cam = CameraMan::getInstance()->getCamera(i);
		string name = Utils::format_string("image%d.jpg", i);
		cam->capture(name.c_str());
	}

	return 0;
}


int _main(int argc, char** argv) 
{
#if 0

	int	retval;
	CameraManager::getInstance()->GetCameraList();
	CameraManager::getInstance()->CreateAllCamera();

	std::vector<CameraThread*> threadlist;

	int len = CameraManager::getInstance()->GetAllCameraLength();
	for (int i = 0; i < len; i++)
	{
		CameraThread* thread = new CameraThread();
		thread->Start(i);
		threadlist.push_back(thread);
	}

	while (true)
	{
		Utils::Sleep(10);

		int i = getch();
		printf("Input = %d\n", i);

		if (i == '1')
		{
			for (int i = 0; i < threadlist.size(); i++)
			{
				threadlist[i]->addTestPacket(PACKET_HALFPRESS, i);
			}
		}
		else if (i == '2')
		{
			for (int i = 0; i < threadlist.size(); i++)
			{
				threadlist[i]->addTestPacket(PACKET_SHOT, i);
			}
		}
	}

#else

	CameraController cont;
	cont.init();

	//int ret = cont.set_settings_value("autofocusdrive", "0");
	int ret1 = cont.set_settings_value("autofocusdrive", 0);

	string v;
	cont.get_settings_value("iso", v);

	int ret = cont.set_settings_value("iso", "400");
	ret = cont.set_settings_value("aperture", "10");
	ret = cont.set_settings_value("shutterspeed", "1/100");

#if 1
	ret = cont.set_settings_value("eosremoterelease", "Press 1");
	printf("set_settings_value : %d\n", ret);

	//ret = cont.set_settings_value("eosremoterelease", "Release 1");
	//printf("set_settings_value : %d\n", ret);
	
	//ret = cont.set_settings_value("eosremoterelease", "Press 1");
	//printf("set_settings_value : %d\n", ret);

#else
	ret = cont.set_settings_value("eosremoterelease", "Press Half");
	printf("set_settings_value : %d\n", ret);

	ret = cont.set_settings_value("eosremoterelease", "Release Half");
	printf("set_settings_value : %d\n", ret);

	ret = cont.set_settings_value("eosremoterelease", "Press Half");
	printf("set_settings_value : %d\n", ret);
#endif

	//string a = "1";
	//int c = cont.capture("1.jpg", a);
	//printf("capture : %d\n", c);

#endif

	cont.release();

	return 0;

}




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