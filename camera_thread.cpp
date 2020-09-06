

#include "camera_context.h"
#include "camera_thread.h"
#include "network.h"
#include "utils.h"

bool CameraThread::exitthread[MAX_CAMERA];
pthread_mutex_t CameraThread::mutex_lock[MAX_CAMERA];
pthread_mutex_t CameraThread::exitmutex_lock[MAX_CAMERA];

CameraThread::CameraThread()
{
	camera_state = CAMERA_STATE::STATE_READY;
}

CameraThread::~CameraThread()
{
}

void CameraThread::Start(int camnum)
{
	camera = CameraManager::getInstance()->GetCameraContext(camnum);

	camera_state[camnum] = CAMERA_STATE::STATE_CONNECTION;

	pthread_mutex_init(&mutex_lock[camnum], NULL);
	pthread_mutex_init(&exitmutex_lock[camnum], NULL);

	int err = pthread_create(&threadid, NULL, thread_fn, (void*)camnum);

}

void* CameraThread::thread_fn(void* arg)
{
	int camnum = (int)arg;

	while (true)
	{
		// check EXIT
		pthread_mutex_lock(&exitmutex_lock[camnum]);

		if (exitthread[camnum])
		{
			pthread_mutex_unlock(&exitmutex_lock[camnum]);
			break;
		}

		pthread_mutex_unlock(&exitmutex_lock[camnum]);

		// Copy display command
		pthread_mutex_lock(&mutex_lock[camnum]);

		Update(camnum);

		pthread_mutex_unlock(&mutex_lock[camnum]);

/*
		if (!_displayinfolist.empty())
		{
			int ret = COMM_NOT_AVAILABLE;
			for (size_t i = 0; i < _displayinfolist.size(); i++)
			{
				Logger::log(LOG_INFO, "Display (%d) %s\n", _displayinfolist[i].command, _displayinfolist[i].param);
				ret = Device::getInstance()->sendcommand(_displayinfolist[i].side, _displayinfolist[i].command,
					_displayinfolist[i].param, _displayinfolist[i].length);
				// 				if (ret != COMM_SUCCESS)
				// 					break;
			}
			_displayinfolist.clear();
			//actionTrigger[_side] = false;
		}
*/

		Utils::Sleep(10);	// 0.1 sec
	}

	pthread_exit((void*)0);
	return((void*)0);
}

void CameraThread::Update(int camnum)
{
	switch (camera_state[camnum])
	{
		case CAMERA_STATE::STATE_CONNECTION:
			{
				network[camnum].init();
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
	}

	network[camnum].update();
}


