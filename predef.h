#ifndef _PREDEF_
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

//#define	TESTBUILD

#define MOTION_JOINT		1
#define MOTION_DISPLAY		2

//////////////////////////////////////////////////////////////////////////

#define PI				3.141592f
//#define USE_RAD
#define RADTODEG(A)		(A * 180.0f / PI);
#define DEGTORAD(A)		(A * PI / 180.0f)

#define LOG_NONE		0
#define LOG_INFO		1 << 2
#define LOG_WARN		1 << 3
#define LOG_ERR			1 << 4

#ifdef TESTBUILD
#define CONSOLE_LOG		LOG_ERR | LOG_WARN | LOG_INFO
#define FILE_LOG		LOG_ERR | LOG_WARN | LOG_INFO
#else
#define CONSOLE_LOG		LOG_ERR | LOG_WARN
#define FILE_LOG		LOG_NONE
#endif


#define ESC_ASCII_VALUE			0x1b
#define PROTOCOL_VERSION		1.0
//#define PROTOCOL_VERSION		2.0

#define DEVICE_DISABLE			0
#define DEVICE_ENABLE			1

#define MX_FIX_ANGLE			0	// 모터간 제어각 수정 값
#define AX_FIX_ANGLE			30	// 모터간 제어각 수정 값

// Control Address
#define TORQUEMODE				24	// 토크 모드
#define TORQUELIMIT				14	// 토크 limit
#define CW_LIMIT_ANGLE			6
#define CCW_LIMIT_ANGLE			8
#define DEST_POSITION			30	// 0 ~ 4096
#define MOVE_SPEED				32	// 0 ~ 1024
#define CURRENT_POSITION		36	// 현재 위치 read

#define D_PARAM					26	// PID - D
#define I_PARAM					27	// PID - I
#define P_PARAM					28	// PID - P


// 0~1023 범위의 값을 사용하면 CCW방향으로 회전하며 0으로 설정하면 정지합니다.
// 1024~2047 범위의 값을 사용하면 CW방향으로 회전하며 1024으로 설정하면 정지합니다.
#define WHEEL_SPEED				32	
// Wheel의 반대쪽 값 변환
#define REVERSEWHEEL(A)			(A+1024)

// part type list
#define PART_TYPE_NONE			0
#define PART_TYPE_DISPLAY		0x10		// e-Ink Display
#define PART_TYPE_JOINT			0x20		// 팔 / 목 / 얼굴 관절
#define PART_TYPE_WHEEL			0x30		// 구동(바퀴) 부


// 동작관련 수치 변형 Macro
#define MX_ROT_VALUE				(11.377)		// 4096 / 360 (360 to 4096)	--> MX-28 용
#define AX_ROT_VALUE				(3.4133)		// 1024 / 300 (300 to 1023)	--> AX-12 용

// #define DEGREE2DXL(A)			(uint16_t)(A*ROT_VALUE)	// degree to dxl value
// #define DXL2DEGREE(A)			(uint16_t)(A/ROT_VALUE)	// dxl value to degree

// 회전 Speed 설정에 필요한 DXL 값 얻기
// N초에 R만큼 회전하려 할때의 RPM / RPM 설정값
// R / (6 * n) = RPM

// R / (0.114 * 6 * n) = RPM 설정값
#define MX_ROTSPEED				0.114
// R / (0.111 * 6 * n) = RPM 설정값
#define AX_ROTSPEED				0.111

// R은 회전량 절대값 <-- ( abs(현재 각도 - 목표 각도) ) 
//#define SPEEDVALUE(N, R)		(uint16_t)(R / (ROTSPEED * 6 * N ))


// animation state		
#define STATE_IDLE			0		// 최초 정지상태
#define STATE_RUNNING		1		// 현재 에니메이션 중
#define STATE_STOPPED		2		// 에니메이션 완료 후 정지

//////////////////////////////////////////////////////////////////////////

// e-ink display

#define	LEFT_EYE		0
#define	RIGHT_EYE		1
#define	EYE_MAX			2

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
