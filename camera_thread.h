
#ifndef _CAMERA_THREAD_
#define _CAMERA_THREAD_

#include "predef.h"

class Network;
class CameraController;
class Command;
class TCP_Socket;
class UDP_Socket;

class CameraThread
{
public :
	CameraThread();
	~CameraThread();

	void Start(int camnum);
	static void* thread_fn(void* arg);

	void addTestPacket(char packet, int camnum);

private:
	static void Update(int camnum);
	static int UpdateCommand(int camnum);

	static int parsePacket(int camnum, char* buf);


	static void StartUpload(int camnum);
	static size_t read_callback(void* ptr, size_t size, size_t nmemb, void* userp);

	pthread_t		threadid;

	static Command command[MAX_CAMERA];
	static Network network[MAX_CAMERA];
	static TCP_Socket tcpsocket[MAX_CAMERA];
	static UDP_Socket udpsocket[MAX_CAMERA];
	static CameraController* cameras[MAX_CAMERA];

	static CAMERA_STATE camera_state[MAX_CAMERA];
	static bool exitthread[MAX_CAMERA];

	static pthread_mutex_t mutex_lock[MAX_CAMERA];
	static pthread_mutex_t exitmutex_lock[MAX_CAMERA];

	static int upload_progress[MAX_CAMERA];

};



#endif