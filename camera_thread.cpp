

#include "camera_context.h"
#include "camera_thread.h"
#include "network.h"
#include "utils.h"
#include "commander.h"
#include "camera_action.h"

#include "CameraController.h"
#include "CameraManager.h"

Command CameraThread::command[MAX_CAMERA];
Network CameraThread::network[MAX_CAMERA];
CameraController* CameraThread::cameras[MAX_CAMERA];
CAMERA_STATE CameraThread::camera_state[MAX_CAMERA];

bool CameraThread::exitthread[MAX_CAMERA];
pthread_mutex_t CameraThread::mutex_lock[MAX_CAMERA];
pthread_mutex_t CameraThread::exitmutex_lock[MAX_CAMERA];

CameraThread::CameraThread()
{
}

CameraThread::~CameraThread()
{
}

void CameraThread::Start(int camnum)
{
	camera_state[camnum] = CAMERA_STATE::STATE_CONNECTION;

	pthread_mutex_init(&mutex_lock[camnum], NULL);
	pthread_mutex_init(&exitmutex_lock[camnum], NULL);

	int err = pthread_create(&threadid, NULL, thread_fn, (void*)camnum);

}

void* CameraThread::thread_fn(void* arg)
{
	int camnum = (int)arg;
	CameraController* camera = CameraMan::getInstance()->getCamera(camnum);
	cameras[camnum] = camera;

	network[camnum].init();
	bool ret = network[camnum].connect();
	if (ret == true)
		camera_state[camnum] = CAMERA_STATE::STATE_READY;

	while (true)
	{
		{
			// check EXIT
			pthread_mutex_lock(&exitmutex_lock[camnum]);

			// 스레드 끝!
			if (exitthread[camnum])
			{
				pthread_mutex_unlock(&exitmutex_lock[camnum]);
				break;
			}

			pthread_mutex_unlock(&exitmutex_lock[camnum]);
		}

		{
			pthread_mutex_lock(&mutex_lock[camnum]);

			// 네트웍 업데이트 / 명령어 처리
			Update(camnum);

			pthread_mutex_unlock(&mutex_lock[camnum]);
		}

		Utils::Sleep(10);	// 0.1 sec
	}


	pthread_exit((void*)0);
	return((void*)0);
}

void CameraThread::Update(int camnum)
{
	// 쌓여 있는 커맨드가 있으면 여기에서 처리한다.
	UpdateCommand(camnum);

/*
	switch (camera_state[camnum])
	{
		case CAMERA_STATE::STATE_CONNECTION:
			{
				bool ret = network[camnum].connect();
				if (ret == true)
					camera_state[camnum] = CAMERA_STATE::STATE_READY;
			}
			break;

		case CAMERA_STATE::STATE_READY:
			break;

		case CAMERA_STATE::STATE_FOCUSING:
			break;

		case CAMERA_STATE::STATE_SHOT:
			break;

		case CAMERA_STATE::STATE_DOWNLOAD:
			break;
	}*/

	// network update / parse packet / 패킷있으면 commander에 추가
	network[camnum].update();
}

int	CameraThread::UpdateCommand(int camnum)
{
	int ret = 0;
	Commander* commander = network[camnum].getcommander();
	if (commander == NULL) return -1;

	std::vector<Command>::iterator it = commander->commandlist.begin();

	for (; it != commander->commandlist.end(); it++)
	{
		switch (it->packet)
		{
			case PACKET_HALFPRESS:
			{
				// 					int p = (int&)*(it->data);
				// 					uint16_t param = p;
				// 					ret = PartController::getInstance()->addsendqueuecommand(lwheelid, MOVE_SPEED, param);

				// 포커스 
				cameras[camnum]->apply_essential_param_param();
				cameras[camnum]->set_settings_value("eosremoterelease", "Press 1");
				printf("End action_camera_wait_focus : %d\n", ret);
			}
			break;

			case PACKET_SHOT :
			{
				// 찍어
				string name = Utils::format_string("name-%d.jpg", camnum);
				int ret = cameras[camnum]->capture(name.c_str());
				printf("Shot : %d\n", camnum);
			}
			break;

			case PACKET_ISO :
			{
				//int value = (int&)*(it->data);
				string value = it->data;
				cameras[camnum]->set_essential_param(CAMERA_PARAM::ISO, value);
			}
			break;

			case PACKET_APERTURE :
			{
				//int value = (int&)*(it->data);
				string value = it->data;
				cameras[camnum]->set_essential_param(CAMERA_PARAM::APERTURE, value);
			}
			break;

			case PACKET_SHUTTERSPEED:
			{
				//int value = (int&)*(it->data);
				string value = it->data;
				cameras[camnum]->set_essential_param(CAMERA_PARAM::SHUTTERSPEED, value);
			}
			break;
		}
	}

	commander->commandlist.clear();
	return ret;
}

void CameraThread::addTestPacket(char packet, int camnum)
{
	pthread_mutex_lock(&mutex_lock[camnum]);

	Commander* commander = network[camnum].getcommander();
	char data[10] = { 1, };
	commander->addcommand(packet, data, 10);

	pthread_mutex_unlock(&mutex_lock[camnum]);

}

