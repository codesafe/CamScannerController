
/*
gphoto2 --force-overwrite --set-config imageformat=3 --set-config iso=0
--set-config eosremoterelease=5 --wait-event-and-download=2s --set-config eosremoterelease=4 --filename image2.jpg


gphoto2 --set-config eosremoterelease=5 --wait-event-and-download=FILEADDED --set-config eosremoterelease=4 --filename image2.jpg


이런식으로 Immediate 사용가능
Choice: 0 None
Choice: 1 Press Half
Choice: 2 Press Full
Choice: 3 Release Half
Choice: 4 Release Full
Choice: 5 Immediate
Choice: 6 Press 1
Choice: 7 Press 2
Choice: 8 Press 3
Choice: 9 Release 1
Choice: 10 Release 2
Choice: 11 Release 3
*/


#if 1



#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "camera_thread.h"
#include "network.h"
#include "utils.h"
#include "commander.h"
#include "udpsocket.h"
#include "tcpsocket.h"

#include "CameraController.h"
#include "CameraManager.h"

Command CameraThread::command[MAX_CAMERA];
Network CameraThread::network[MAX_CAMERA];

TCP_Socket CameraThread::tcpsocket[MAX_CAMERA];
UDP_Socket CameraThread::udpsocket[MAX_CAMERA];

CameraController* CameraThread::cameras[MAX_CAMERA];
CAMERA_STATE CameraThread::camera_state[MAX_CAMERA];

bool CameraThread::exitthread[MAX_CAMERA];
pthread_mutex_t CameraThread::mutex_lock[MAX_CAMERA];
pthread_mutex_t CameraThread::exitmutex_lock[MAX_CAMERA];
int CameraThread::upload_progress[MAX_CAMERA];


CameraThread::CameraThread()
{
}

CameraThread::~CameraThread()
{
}

void CameraThread::Start(int camnum)
{
	camera_state[camnum] = CAMERA_STATE::STATE_STARTCONNECT;

	pthread_mutex_init(&mutex_lock[camnum], NULL);
	pthread_mutex_init(&exitmutex_lock[camnum], NULL);

	int err = pthread_create(&threadid, NULL, thread_fn, (void*)camnum);

}

void* CameraThread::thread_fn(void* arg)
{
	int camnum = (int)arg;
	CameraController* camera = CameraMan::getInstance()->getCamera(camnum);
	if (camera == NULL)
	{
		Logger::log(-1, "Thread : Camera Create fail");
		return((void*)0);
	}
	else
	{
		Logger::log(-1, "Thread : Camera Create success %d", camnum);
	}

	cameras[camnum] = camera;

//	network[camnum].init();
	tcpsocket[camnum].init();
//	udpsocket[camnum].init(camnum);

	while (true)
	{
		/*
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
		*/

		{
			pthread_mutex_lock(&mutex_lock[camnum]);

			// 네트웍 업데이트 / 명령어 처리
			Update(camnum);

			pthread_mutex_unlock(&mutex_lock[camnum]);
		}

		Utils::Sleep(0);
	}


	pthread_exit((void*)0);
	return((void*)0);
}

void CameraThread::Update(int camnum)
{
	switch (camera_state[camnum])
	{
		case CAMERA_STATE::STATE_CONNECT_ERROR:
			Logger::log(camnum, "STATE STATE_CONNECT_ERROR. Retry Connect.");
			Utils::Sleep(2);
			camera_state[camnum] = CAMERA_STATE::STATE_STARTCONNECT;
			return;

		case CAMERA_STATE::STATE_STARTCONNECT:
			{
				Logger::log(camnum, "Start Connect to Server.");
				bool connected = tcpsocket[camnum].connect();
				if (connected)
					camera_state[camnum] = CAMERA_STATE::STATE_CONNECTION;
				else
					camera_state[camnum] = CAMERA_STATE::STATE_CONNECT_ERROR;
			}
			break;

		case CAMERA_STATE::STATE_CONNECTION:
			{
				char buf[TCP_BUFFER];
				int recvbyte = tcpsocket[camnum].recv(buf);
				if (recvbyte > 0)
				{
					char _num = buf[0];
					udpsocket[camnum].init(_num);
					camera_state[camnum] = CAMERA_STATE::STATE_READY;
					Logger::log(camnum, "Connected! Created Camera");
				}
			}
			break;

		case CAMERA_STATE::STATE_READY:
			{
				char buf[UDP_BUFFER];
				int ret = udpsocket[camnum].update(buf);
				if (ret > 0)
					parsePacket(camnum, buf);
				else
					Logger::log(camnum, "Recv UDP error");
			}
			break;

		case CAMERA_STATE::STATE_FOCUSING:
		case CAMERA_STATE::STATE_SHOT:
			break;

		case CAMERA_STATE::STATE_UPLOAD:
		{
			StartUpload(camnum);
			return;
		}
		break;

		case CAMERA_STATE::STATE_UPLOADING:
		{
			if (upload_progress[camnum] == 10)
			{
				char data[10];
				network[camnum].write(PACKET_UPLOAD_DONE, data, 10);
				camera_state[camnum] = CAMERA_STATE::STATE_READY;
			}
			return;
		}
		break;
	}

	//Logger::log(0,"Camera %d State : %d", camnum, camera_state[camnum]);
	// network update / parse packet / 패킷있으면 commander에 추가
	//network[camnum].update();
	// 쌓여 있는 커맨드가 있으면 여기에서 처리한다.
	//UpdateCommand(camnum);


}


/*
#define 	GP_ERROR_CORRUPTED_DATA   -102
Corrupted data received.More...
#define 	GP_ERROR_FILE_EXISTS   -103
File already exists.More...
#define 	GP_ERROR_MODEL_NOT_FOUND   -105
Specified camera model was not found.More...
#define 	GP_ERROR_DIRECTORY_NOT_FOUND   -107
Specified directory was not found.More...
#define 	GP_ERROR_FILE_NOT_FOUND   -108
Specified file was not found.More...
#define 	GP_ERROR_DIRECTORY_EXISTS   -109
Specified directory already exists.More...
#define 	GP_ERROR_CAMERA_BUSY   -110
The camera is already busy.More...
#define 	GP_ERROR_PATH_NOT_ABSOLUTE   -111
Path is not absolute.More...
#define 	GP_ERROR_CANCEL   -112
Cancellation successful.More...
#define 	GP_ERROR_CAMERA_ERROR   -113
Unspecified camera error.More...
#define 	GP_ERROR_OS_FAILURE   -114
Unspecified failure of the operating system.More...
#define 	GP_ERROR_NO_SPACE   -115
Not enough space.More...
*/

string GetError(int errorcode)
{
	switch (errorcode)
	{
		case GP_ERROR_CORRUPTED_DATA:
			return string("GP_ERROR_CORRUPTED_DATA");

		case GP_ERROR_FILE_EXISTS:
			return string("GP_ERROR_FILE_EXISTS");
		case GP_ERROR_MODEL_NOT_FOUND:
			return string("GP_ERROR_MODEL_NOT_FOUND");
		case GP_ERROR_DIRECTORY_NOT_FOUND:
			return string("GP_ERROR_DIRECTORY_NOT_FOUND");
		case GP_ERROR_FILE_NOT_FOUND:
			return string("GP_ERROR_FILE_NOT_FOUND");
		case GP_ERROR_DIRECTORY_EXISTS:
			return string("GP_ERROR_DIRECTORY_EXISTS");
		case GP_ERROR_CAMERA_BUSY:
			return string("GP_ERROR_CAMERA_BUSY");
		case GP_ERROR_PATH_NOT_ABSOLUTE:
			return string("GP_ERROR_PATH_NOT_ABSOLUTE");
		case GP_ERROR_CANCEL:
			return string("GP_ERROR_CANCEL");
		case GP_ERROR_CAMERA_ERROR:
			return string("GP_ERROR_CAMERA_ERROR");
		case GP_ERROR_OS_FAILURE:
			return string("GP_ERROR_OS_FAILURE");
		case GP_ERROR_NO_SPACE:
			return string("GP_ERROR_NO_SPACE");
	}
		
	string errorstr = gp_port_result_as_string(errorcode);
	return errorstr;
}

int CameraThread::parsePacket(int camnum, char* buf)
{
	char packet = buf[0];
	int ret = 0;
	std::string  date = Utils::getCurrentDateTime();

	switch (packet)
	{
		case PACKET_TRY_CONNECT:
			break;

		case PACKET_HALFPRESS:
		{

			char data[TCP_BUFFER] = { 0, };
			data[0] = PACKET_AUTOFOCUS_RESULT;

/*
			if (cameras[camnum]->is_halfpressed())
			{
				ret = cameras[camnum]->set_settings_value("eosremoterelease", "Release Full");
				if (ret < GP_OK)
				{
					printf("ERR eosremoterelease Release Full : %d : %d\n", ret, camnum);
					return ret;
				}
				printf("End Release 1 : %d : %d\n", ret, camnum);
			}
*/
			char i = buf[1];	// iso
			char s = buf[2];	// shutterspeed
			char a = buf[3];	// aperture

			cameras[camnum]->set_essential_param(CAMERA_PARAM::ISO, isoString[i]);
			cameras[camnum]->set_essential_param(CAMERA_PARAM::SHUTTERSPEED, shutterspeedString[s]);
			cameras[camnum]->set_essential_param(CAMERA_PARAM::APERTURE, apertureString[a]);

			ret = cameras[camnum]->apply_autofocus(camnum, false);
			if (ret < GP_OK)
			{
				string errorstr = GetError(ret);
				Logger::log(camnum,"ERR apply_autofocus (False) : %s(%d): %d\n", errorstr.c_str(), ret, camnum);

				data[1] = RESPONSE_FAIL;
				tcpsocket[camnum].send(data);
				return ret;
			}

			ret = cameras[camnum]->apply_essential_param_param(camnum);
			if (ret < GP_OK)
			{
				string errorstr = GetError(ret);
				Logger::log(camnum, "ERR apply_essential_param_param : %s(%d): %d\n", errorstr.c_str(), ret, camnum);

				data[1] = RESPONSE_FAIL;
				tcpsocket[camnum].send(data);
				return ret;
			}

			// 포커스 
			ret = cameras[camnum]->set_settings_value("eosremoterelease", "Press Half");
			if (ret < GP_OK)
			{
				string errorstr = GetError(ret);
				Logger::log(camnum, "ERR eosremoterelease Press Half : %s(%d): %d\n", errorstr.c_str(), ret, camnum);


				data[1] = RESPONSE_FAIL;
				tcpsocket[camnum].send(data);
				return ret;
			}
			else
				Logger::log(camnum, "Press Half : %d : %d\n", ret, camnum);

			ret = cameras[camnum]->set_settings_value("eosremoterelease", "Release Full");
			if (ret < GP_OK)
			{
				string errorstr = GetError(ret);
				Logger::log(camnum, "ERR eosremoterelease Release Full : %s(%d): %d\n", errorstr.c_str(), ret, camnum);

				data[1] = RESPONSE_FAIL;
				tcpsocket[camnum].send(data);
				return ret;
			}

			ret = cameras[camnum]->apply_autofocus(camnum, false);
			if (ret < GP_OK)
			{
				string errorstr = GetError(ret);
				Logger::log(camnum, "ERR apply_autofocus (False) : %s(%d): %d\n", errorstr.c_str(), ret, camnum);

				data[1] = RESPONSE_FAIL;
				tcpsocket[camnum].send(data);
				return ret;
			}

			data[1] = RESPONSE_OK;
			tcpsocket[camnum].send(data);
		}
		break;

		case PACKET_SHOT:
		{
			Logger::log(camnum, "---> Shot : %s", date.c_str());
			// 찍어
			string name = Utils::format_string("name-%d.jpg", camnum);
			int ret = cameras[camnum]->capture3(name.c_str());

			if (ret < GP_OK)
			{
				camera_state[camnum] = CAMERA_STATE::STATE_READY;
				Logger::log(camnum, "Shot Error : (%d)", ret);
			}
			else
			{
				std::string  date = Utils::getCurrentDateTime();
				Utils::Sleep(1);
				//int ret = cameras[camnum]->downloadimage(name.c_str());
				camera_state[camnum] = CAMERA_STATE::STATE_UPLOAD;
			}
		}
		break;

		case PACKET_ISO:
		{
			//int value = (int&)*(it->data);
			//string value = (char*)buf[1];
			char v = buf[1];
			iso = isoString[v];
			cameras[camnum]->set_essential_param(CAMERA_PARAM::ISO, iso);
		}
		break;

		case PACKET_APERTURE:
		{
			//int value = (int&)*(it->data);
			char v = buf[1];
			aperture = apertureString[v];
			cameras[camnum]->set_essential_param(CAMERA_PARAM::APERTURE, apertureString[v]);
		}
		break;

		case PACKET_SHUTTERSPEED:
		{
			//int value = (int&)*(it->data);
			char v = buf[1];
			shutterspeed = shutterspeedString[v];
			cameras[camnum]->set_essential_param(CAMERA_PARAM::SHUTTERSPEED, shutterspeed);
		}
		break;

		case PACKET_FORCE_UPLOAD:
			//camera_state[camnum] = CAMERA_STATE::STATE_UPLOAD;
			break;
	}

	return ret;
}


int	CameraThread::UpdateCommand(int camnum)
{
	int ret = 0;
	Commander* commander = network[camnum].getcommander();
	if (commander == NULL) return -1;

	std::string  date = Utils::getCurrentDateTime();
	std::vector<Command>::iterator it = commander->commandlist.begin();

	for (; it != commander->commandlist.end(); it++)
	{
		switch (it->packet)
		{
		case PACKET_TRY_CONNECT:
			camera_state[camnum] = CAMERA_STATE::STATE_CONNECTION;
			break;

		case PACKET_HALFPRESS:
		{
/*
			if (cameras[camnum]->is_halfpressed())
			{
				ret = cameras[camnum]->set_settings_value("eosremoterelease", "Release Full");
				if (ret < GP_OK)
				{
					printf("ERR eosremoterelease Release Full : %d : %d\n", ret, camnum);
					return ret;
				}
				printf("End Release 1 : %d : %d\n", ret, camnum);
			}
*/

			// 					int p = (int&)*(it->data);
			// 					uint16_t param = p;
			// 					ret = PartController::getInstance()->addsendqueuecommand(lwheelid, MOVE_SPEED, param);

			ret = cameras[camnum]->apply_autofocus(camnum, true);
			if (ret < GP_OK)
			{
				printf("ERR apply_autofocus (True) : %d : %d\n", ret, camnum);
				return ret;
			}

			ret = cameras[camnum]->apply_essential_param_param(camnum);
			if (ret < GP_OK)
			{
				printf("ERR apply_essential_param_param : %d : %d\n", ret, camnum);
				return ret;
			}

			// 포커스 
			ret = cameras[camnum]->set_settings_value("eosremoterelease", "Press Half");
			if (ret < GP_OK)
			{
				printf("ERR eosremoterelease Press Half : %d : %d\n", ret, camnum);
				return ret;
			}
			else
				printf("Press Half : %d : %d\n", ret, camnum);

			ret = cameras[camnum]->apply_autofocus(camnum, false);
			if (ret < GP_OK)
			{
				printf("ERR apply_autofocus (False) : %d : %d\n", ret, camnum);
				return ret;
			}

		}
		break;

		case PACKET_SHOT:
		{
			printf("---> Shot : %d %s \n", camnum, date.c_str());
			// 찍어
			string name = Utils::format_string("name-%d.jpg", camnum);
			int ret = cameras[camnum]->capture3(name.c_str());

			if (ret < GP_OK)
			{
				camera_state[camnum] = CAMERA_STATE::STATE_READY;
				printf("Shot : %d Error : (%d)\n", camnum, ret);
			}
			else
			{
				std::string  date = Utils::getCurrentDateTime();
				Utils::Sleep(1);
				//int ret = cameras[camnum]->downloadimage(name.c_str());
				camera_state[camnum] = CAMERA_STATE::STATE_UPLOAD;
			}
		}
		break;

		case PACKET_ISO:
		{
			//int value = (int&)*(it->data);
			string value = it->data;
			cameras[camnum]->set_essential_param(CAMERA_PARAM::ISO, value);
		}
		break;

		case PACKET_APERTURE:
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

		case PACKET_FORCE_UPLOAD:
			camera_state[camnum] = CAMERA_STATE::STATE_UPLOAD;
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

void CameraThread::StartUpload(int camnum)
{
	Logger::log(camnum, "Start Upload FTP");
	camera_state[camnum] = CAMERA_STATE::STATE_UPLOADING;
	upload_progress[camnum] = 0;

	CURL* curl;
	CURLcode res;
	struct WriteThis upload;

	string name = Utils::format_string("name-%d.jpg", camnum);
	char* inbuf = NULL;
	int len = 0;

	FILE* fp = NULL;
	fp = fopen(name.c_str(), "rb");
	if (fp == NULL)
	{
		Logger::log(camnum, "fp is NULL");
		camera_state[camnum] = CAMERA_STATE::STATE_READY;
		return;
	}

	fseek(fp, 0, SEEK_END);
	len = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	inbuf = new char[len];
	fread(inbuf, 1, len, fp);
	fclose(fp);

	upload.camnum = camnum;
	upload.readptr = inbuf;
	upload.totalsize = len;
	upload.sizeleft = len;

	Logger::log(camnum, "filesize : %d",len);


	curl = curl_easy_init();
	if (curl)
	{
		string url = "ftp://192.168.29.103/" + name;
		//curl_easy_setopt(curl, CURLOPT_URL, "ftp://192.168.29.103/2.jpg");
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_USERPWD, "codesafe:6502");
		curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
		curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
		curl_easy_setopt(curl, CURLOPT_READDATA, &upload);
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
		curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)upload.sizeleft);

		// 전송!
		res = curl_easy_perform(curl);

		if (res != CURLE_OK)
		{
			Logger::log(camnum, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		}
		curl_easy_cleanup(curl);
		Logger::log(camnum, "Upload complete\n");
	}

	// 끝
	delete[] inbuf;
}

size_t CameraThread::read_callback(void* ptr, size_t size, size_t nmemb, void* userp)
{
	struct WriteThis* upload = (struct WriteThis*)userp;
	size_t max = size * nmemb;

	if (max < 1)
		return 0;

	if (upload->sizeleft)
	{
		size_t copylen = max;
		if (copylen > upload->sizeleft)
			copylen = upload->sizeleft;
		memcpy(ptr, upload->readptr, copylen);
		upload->readptr += copylen;
		upload->sizeleft -= copylen;

		int progress = (int)(((float)(upload->totalsize - upload->sizeleft) / upload->totalsize) * 100.f);
		int p = (progress / 10);

		if (upload_progress[upload->camnum] != p)
		{
			upload_progress[upload->camnum] = p;
			Logger::log(upload->camnum, "upload_progress %d\n", upload_progress[upload->camnum]);

			//char data[32];
			//memcpy(data, &upload_progress[upload->camnum], sizeof(int));
			//network[upload->camnum].write(PACKET_UPLOAD_PROGRESS, data, 32);
			//network[upload->camnum].update();

			char buf[TCP_BUFFER];
			if (p == 10)
			{
				buf[0] = PACKET_UPLOAD_DONE;
			}
			else
			{
				buf[0] = PACKET_UPLOAD_PROGRESS;
				buf[1] = (char)upload_progress[upload->camnum];
			}

			tcpsocket[upload->camnum].send(buf);
		}
		//printf("Progress : %d\n", progress);
		return copylen;
	}

	return 0;                          /* no more data left to deliver */
}


#else

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

//#include "camera_context.h"
#include "camera_thread.h"
#include "network.h"
#include "utils.h"
#include "commander.h"
//#include "camera_action.h"
#include "udpsocket.h"

#include "CameraController.h"
#include "CameraManager.h"

Command CameraThread::command[MAX_CAMERA];
Network CameraThread::network[MAX_CAMERA];
UDP_Socket CameraThread::udpsocket[MAX_CAMERA];

CameraController* CameraThread::cameras[MAX_CAMERA];
CAMERA_STATE CameraThread::camera_state[MAX_CAMERA];

bool CameraThread::exitthread[MAX_CAMERA];
pthread_mutex_t CameraThread::mutex_lock[MAX_CAMERA];
pthread_mutex_t CameraThread::exitmutex_lock[MAX_CAMERA];
int CameraThread::upload_progress[MAX_CAMERA];


CameraThread::CameraThread()
{
}

CameraThread::~CameraThread()
{
}

void CameraThread::Start(int camnum)
{
	camera_state[camnum] = CAMERA_STATE::STATE_CONNECTION;
	//camera_state[camnum] = CAMERA_STATE::STATE_READY;

	pthread_mutex_init(&mutex_lock[camnum], NULL);
	pthread_mutex_init(&exitmutex_lock[camnum], NULL);

	int err = pthread_create(&threadid, NULL, thread_fn, (void*)camnum);

}

void* CameraThread::thread_fn(void* arg)
{
	int camnum = (int)arg;
	CameraController* camera = CameraMan::getInstance()->getCamera(camnum);
	if (camera == NULL)
	{
		Logger::log(0, "Thread : Camera Create fail");
		return((void*)0);
	}
	else
	{
		Logger::log(0, "Thread : Camera Create success %d", camnum);
	}

	cameras[camnum] = camera;

 	network[camnum].init();
	udpsocket[camnum].init(camnum);


// 	bool ret = network[camnum].connect();
// 	if (ret == true)
// 		camera_state[camnum] = CAMERA_STATE::STATE_READY;

	while (true)
	{
/*
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
*/

		{
			pthread_mutex_lock(&mutex_lock[camnum]);

			// 네트웍 업데이트 / 명령어 처리
			Update(camnum);

			pthread_mutex_unlock(&mutex_lock[camnum]);
		}

		Utils::Sleep(0.01f);
	}


	pthread_exit((void*)0);
	return((void*)0);
}

void CameraThread::Update(int camnum)
{
	// 쌓여 있는 커맨드가 있으면 여기에서 처리한다.
	UpdateCommand(camnum);

	switch (camera_state[camnum])
	{
		case CAMERA_STATE::STATE_CONNECTION:
			{
				bool ret = network[camnum].connect(camnum);
				camera_state[camnum] = CAMERA_STATE::STATE_READY;
			}
			break;

		case CAMERA_STATE::STATE_READY:
			break;

		case CAMERA_STATE::STATE_FOCUSING:
			break;

		case CAMERA_STATE::STATE_SHOT:
			break;

		case CAMERA_STATE::STATE_UPLOAD:
			{
				StartUpload(camnum);
			}
			break;

		case CAMERA_STATE::STATE_UPLOADING:
			{
				if (upload_progress[camnum] == 10)
				{
					char data[10];
					network[camnum].write(PACKET_UPLOAD_DONE, data, 10);
					camera_state[camnum] = CAMERA_STATE::STATE_READY;
				}

			}
			break;
	}

	char command = udpsocket[camnum].update();

	// network update / parse packet / 패킷있으면 commander에 추가
	//network[camnum].update();

	std::string  date = Utils::getCurrentDateTime();

	if (command == 0)
	{
		int ret = 0;
		string name = Utils::format_string("name-%d.jpg", camnum);
		cameras[camnum]->capture3(name.c_str());
		//cameras[camnum]->capture3("capt0000.jpg");
		printf("End Release 1 : %d : %d\n", ret, camnum);
		printf("---> Option : %d %s \n", camnum, date.c_str());
	}
	else if (command == 1)
	{
		printf("---> Shot : %d %s \n", camnum, date.c_str());
		// 찍어
		string name = Utils::format_string("name-%d.jpg", camnum);
		//int ret = cameras[camnum]->capture2(name.c_str());

		int ret = cameras[camnum]->set_settings_value("eosremoterelease", "Immediate");

		if (ret < GP_OK)
		{
			camera_state[camnum] = CAMERA_STATE::STATE_READY;
			printf("Shot : %d Error : (%d)\n", camnum, ret);
		}
		else
		{
			std::string  date = Utils::getCurrentDateTime();
			printf("---> Shot : %d %s \n", camnum, date.c_str());

			Utils::Sleep(5);

			int ret = cameras[camnum]->downloadimage(name.c_str());

			ret = cameras[camnum]->set_settings_value("eosremoterelease", "Release Full");

			camera_state[camnum] = CAMERA_STATE::STATE_UPLOAD;
		}
	}
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
			case PACKET_TRY_CONNECT:
				camera_state[camnum] = CAMERA_STATE::STATE_CONNECTION;
				break;

			case PACKET_HALFPRESS:
			{
				// 					int p = (int&)*(it->data);
				// 					uint16_t param = p;
				// 					ret = PartController::getInstance()->addsendqueuecommand(lwheelid, MOVE_SPEED, param);

				// 포커스 
				ret = cameras[camnum]->apply_essential_param_param(camnum);
				if (ret < GP_OK)
				{
					printf("ERR apply_essential_param_param : %d : %d\n", ret, camnum);
					return ret;
				}

				ret = cameras[camnum]->set_settings_value("eosremoterelease", "Press 1");
				if (ret < GP_OK)
				{
					printf("ERR eosremoterelease Press 1 : %d : %d\n", ret, camnum);
					return ret;
				}

				printf("End Press 1 : %d : %d\n", ret, camnum);
				ret = cameras[camnum]->set_settings_value("eosremoterelease", "Release 1");
				if (ret < GP_OK)
				{
					printf("ERR eosremoterelease Release 1 : %d : %d\n", ret, camnum);
					return ret;
				}

				printf("End Release 1 : %d : %d\n", ret, camnum);
			}
			break;

			case PACKET_SHOT :
			{
				// 찍어
				string name = Utils::format_string("name-%d.jpg", camnum);
				int ret = cameras[camnum]->capture2(name.c_str());

				if (ret < GP_OK)
				{
					camera_state[camnum] = CAMERA_STATE::STATE_READY;
					printf("Shot : %d Error : (%d)\n", camnum, ret);
				}
				else
				{
					std::string  date = Utils::getCurrentDateTime();
					printf("---> Shot : %d %s \n", camnum, date.c_str());

					Utils::Sleep(5);

					int ret = cameras[camnum]->downloadimage(name.c_str());

					camera_state[camnum] = CAMERA_STATE::STATE_UPLOAD;
				}
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

			case PACKET_FORCE_UPLOAD:
				camera_state[camnum] = CAMERA_STATE::STATE_UPLOAD;
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

void CameraThread::StartUpload(int camnum)
{
	camera_state[camnum] = CAMERA_STATE::STATE_UPLOADING;
	upload_progress[camnum] = 0;

	CURL* curl;
	CURLcode res;
	struct WriteThis upload;

	string name = Utils::format_string("name-%d.jpg", camnum);
	char* inbuf = NULL;
	int len = 0;

	FILE* fp = NULL;
	fp = fopen(name.c_str(), "rb");
	if (fp == NULL)
	{
		camera_state[camnum] = CAMERA_STATE::STATE_READY;
		return;
	}

	fseek(fp, 0, SEEK_END);
	len = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	inbuf = new char[len];
	fread(inbuf, 1, len, fp);
	fclose(fp);

	upload.camnum = camnum;
	upload.readptr = inbuf;
	upload.totalsize = len;
	upload.sizeleft = len;


	curl = curl_easy_init();
	if (curl)
	{
		string url = "ftp://192.168.29.103/" + name;
		//curl_easy_setopt(curl, CURLOPT_URL, "ftp://192.168.29.103/2.jpg");
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_USERPWD, "codesafe:6502");
		curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
		curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
		curl_easy_setopt(curl, CURLOPT_READDATA, &upload);
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
		curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)upload.sizeleft);

		// 전송!
		res = curl_easy_perform(curl);

		if (res != CURLE_OK)
		{
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		}
		curl_easy_cleanup(curl);

		printf("Upload complete\n");
	}

	// 끝
	delete [] inbuf;
}

size_t CameraThread::read_callback(void* ptr, size_t size, size_t nmemb, void* userp)
{
	struct WriteThis* upload = (struct WriteThis*)userp;
	size_t max = size * nmemb;

	if (max < 1)
		return 0;

	if (upload->sizeleft)
	{
		size_t copylen = max;
		if (copylen > upload->sizeleft)
			copylen = upload->sizeleft;
		memcpy(ptr, upload->readptr, copylen);
		upload->readptr += copylen;
		upload->sizeleft -= copylen;

		int progress = (int)( ((float)(upload->totalsize-upload->sizeleft) / upload->totalsize) * 100.f );
		int p = (progress / 10);

		if (upload_progress[upload->camnum] != p)
		{
			upload_progress[upload->camnum] = p;
			printf("upload_progress %d\n", upload_progress[upload->camnum]);

			char data[32];
			memcpy(data, &upload_progress[upload->camnum], sizeof(int));

			network[upload->camnum].write(PACKET_UPLOAD_PROGRESS, data, 32);
			network[upload->camnum].update();
		}
		//printf("Progress : %d\n", progress);
		return copylen;
	}

	return 0;                          /* no more data left to deliver */
}

#endif