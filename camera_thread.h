
#ifndef _CAMERA_THREAD_
#define _CAMERA_THREAD_

#include "predef.h"

class Network;
class CameraController;
class Command;

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

	pthread_t		threadid;

	static Command command[MAX_CAMERA];
	static Network network[MAX_CAMERA];
	static CameraController* cameras[MAX_CAMERA];

	static CAMERA_STATE camera_state[MAX_CAMERA];
	static bool exitthread[MAX_CAMERA];


	static pthread_mutex_t mutex_lock[MAX_CAMERA];
	static pthread_mutex_t exitmutex_lock[MAX_CAMERA];
};



#endif