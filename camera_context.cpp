
#include <stdio.h>
//#include <gphoto2/gphoto2-camera.h>
#include "camera_context.h"

static void ctx_error_func(GPContext* context, const char* str, void* data)
{
	fprintf(stderr, "\n*** Contexterror ***              \n%s\n", str);
	fflush(stderr);
}

static void ctx_status_func(GPContext* context, const char* str, void* data)
{
	fprintf(stderr, "%s\n", str);
	fflush(stderr);
}

//////////////////////////////////////////////////////////////////////////


cameracontext* cameracontext::_instance = nullptr;

cameracontext::cameracontext()
{
}

cameracontext::~cameracontext()
{
}

GPContext* cameracontext::getcontext()
{
	GPContext* context;

	/* This is the mandatory part */
	context = gp_context_new();

	/* All the parts below are optional! */
	gp_context_set_error_func(context, ctx_error_func, NULL);
	gp_context_set_status_func(context, ctx_status_func, NULL);

	/* also:
	gp_context_set_cancel_func    (p->context, ctx_cancel_func,  p);
		gp_context_set_message_func   (p->context, ctx_message_func, p);
		if (isatty (STDOUT_FILENO))
				gp_context_set_progress_funcs (p->context,
						ctx_progress_start_func, ctx_progress_update_func,
						ctx_progress_stop_func, p);
		*/
	return context;
}