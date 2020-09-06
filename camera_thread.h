
#ifndef _CAMERA_THREAD_
#define _CAMERA_THREAD_

#include "predef.h"

class Network;
class Camera_Context;

class CameraThread
{
public :
	CameraThread();
	~CameraThread();

	void Start(int camnum);
	static void* thread_fn(void* arg);

private:
	static void Update(int camnum);

	Camera_Context* camera;
	pthread_t		threadid;

	static Network network[MAX_CAMERA];
	static CAMERA_STATE camera_state[MAX_CAMERA];
	static bool exitthread[MAX_CAMERA];
	static pthread_mutex_t mutex_lock[MAX_CAMERA];
	static pthread_mutex_t exitmutex_lock[MAX_CAMERA];
};



#endif