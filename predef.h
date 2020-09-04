﻿#ifndef _PREDEF_
#define _PREDEF_

#ifdef __linux__
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/usbdevice_fs.h>
#elif defined(_WIN32) || defined(_WIN64)
#include <conio.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <ctime>
#include <deque>
#include <string.h> 

#include "memdb.h"
#include "logger.h"

//using namespace std;
//using namespace dynamixel;

#ifdef WIN32
#  define thread_local __declspec(thread) 
#else
#  define thread_local __thread
#endif

//////////////////////////////////////////////////////////////////////////

#include <gphoto2/gphoto2-camera.h>
#include <gphoto2/gphoto2-abilities-list.h>
#include <gphoto2/gphoto2-context.h>

typedef struct _GPParams GPParams;
struct _GPParams 
{
	Camera* camera;
	GPContext* context;
};

enum wait_type 
{
	WAIT_TIME,
	WAIT_EVENTS,
	WAIT_FRAMES,
	WAIT_STRING,
};

enum download_type { DT_NO_DOWNLOAD, DT_DOWNLOAD };
struct waitparams
{
	union {
		int milliseconds;
		int events;
		int frames;
		char* str;
	} u;
	enum wait_type type;
	enum download_type downloadtype;
};


//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////

//#define	USE_NONEBLOCK

#define PATCHDOWNLOADDIR	"./tempdownload"
#define PATCHSERVER_ADD		"http://127.0.0.1:8000/patch/patch.xml"
#define PATCHFILENAME		"patch.xml"

#define SERVER_ADD			"127.0.0.1"
#define SERVER_PORT			8888
#define SOCKET_BUFFER		4096

//#define I_AM_SERVER

//////////////////////////////////////////////////////////////////////////

// network packet

// response packet
#define RESPONSE_OK		0x05
#define RESPONSE_FAIL	0x06

// Log packet
#define CLIENT_LOG_INFO		0x0a
#define CLIENT_LOG_WARN		0x0b
#define CLIENT_LOG_ERR		0x0c

// command packet
#define FORCEPATCH			0x0A
#define DEVICERESET			0x10		// 재시작

#define	PLAYMOTION			0x20

#define WHEEL_FORWARD		0x30
#define WHEEL_BACKWARD		0x31
#define WHEEL_STOP			0x32
#define WHEEL_TURNLEFT		0x33
#define WHEEL_TURNRIGHT		0x34

#define DISPLAY_PIC			0x40


// Command type
#define COMMAND_FORCEPATCH	0x0A	// force online patch and reset system
#define COMMAND_DEVICE		0x10
#define COMMAND_ANIMATION	0x20
#define COMMAND_WHEEL		0x30
#define COMMAND_DISPLAY		0x40





#endif