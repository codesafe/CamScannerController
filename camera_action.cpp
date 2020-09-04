#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <string>
#include <vector>
#include <sys/time.h>
#include <utime.h>

#include "predef.h"
#include "./camera_action.h"

Camera_Action::Camera_Action()
{
}
Camera_Action::~Camera_Action()
{
}


int Camera_Action::get_all_config(GPParams* p)
{
	CameraWidget* rootconfig;
	int	ret;

	ret = gp_camera_get_config(p->camera, &rootconfig, p->context);
	if (ret != GP_OK) return ret;
	//display_widgets(p, rootconfig, "", 1);
	gp_widget_free(rootconfig);
	return (GP_OK);
}

// 카메라 셋팅할수있는 값들을 얻는다 (enum)
int Camera_Action::get_config(GPParams* p, char* name)
{
	CameraWidget* rootconfig, * child;
	int	ret = GP_OK;

	ret = _find_widget_by_name(p, name, &child, &rootconfig);
	if (ret != GP_OK)
		return ret;

	//ret = print_widget(p, name, child);
	ret = get_widget(p, name, child);

	gp_widget_free(rootconfig);
	return ret;
}

// 특정 카메라 셋팅 값을 얻는다
int Camera_Action::get_camera_config(GPParams* p, char* name)
{
	CameraWidget* rootconfig, * child;
	int	ret = GP_OK;

	ret = _find_widget_by_name(p, name, &child, &rootconfig);
	if (ret != GP_OK)
		return ret;

	//ret = print_widget(p, name, child);

	char* current;
	ret = gp_widget_get_value(child, &current);
	if (ret == GP_OK)
	{
		gp_widget_count_choices(child);
		printf("Current: %s\n", current);
	}

// 	int index;
// 	ret = gp_widget_get_id(child, &index);

	gp_widget_free(rootconfig);
	return ret;
}

/*
	ex) --set-config-index iso=10
	Value랑은 다르다
*/
int Camera_Action::set_camera_config(GPParams* p, char *name, int value)
{
	CameraWidget* rootconfig, * child;
	int	ret;
	const char* label;
	CameraWidgetType	type;

	ret = _find_widget_by_name(p, name, &child, &rootconfig);
	if (ret != GP_OK)
		return ret;

	ret = gp_widget_get_type(child, &type);
	if (ret != GP_OK) 
	{
		gp_widget_free(rootconfig);
		return ret;
	}

	ret = gp_widget_get_label(child, &label);
	if (ret != GP_OK) 
	{
		gp_widget_free(rootconfig);
		return ret;
	}

	switch (type) 
	{
		case GP_WIDGET_MENU:
		case GP_WIDGET_RADIO: { /* char *		*/
			int cnt, i = value;

			cnt = gp_widget_count_choices(child);
			if (cnt < GP_OK) 
			{
				ret = cnt;
				break;
			}
			ret = GP_ERROR_BAD_PARAMETERS;

			if ((i >= 0) && (i < cnt)) 
			{
				const char* choice;
				ret = gp_widget_get_choice(child, i, &choice);
				if (ret == GP_OK)
					ret = gp_widget_set_value(child, choice);
				break;
			}
			gp_context_error(p->context, "Choice %s not found within list of choices.", value);
			break;
		}

		/* ignore: */
		case GP_WIDGET_TOGGLE:
		case GP_WIDGET_TEXT:
		case GP_WIDGET_RANGE:
		case GP_WIDGET_DATE:
		case GP_WIDGET_WINDOW:
		case GP_WIDGET_SECTION:
		case GP_WIDGET_BUTTON:
			gp_context_error(p->context, "The %s widget has no indexed list of choices. Use --set-config-value instead.", name);
			ret = GP_ERROR_BAD_PARAMETERS;
			break;
	}

	if (ret == GP_OK) 
	{
		if (child == rootconfig)
			ret = gp_camera_set_single_config(p->camera, name, child, p->context);
		else
			ret = gp_camera_set_config(p->camera, rootconfig, p->context);
		if (ret != GP_OK)
			gp_context_error(p->context, "Failed to set new configuration value %s for configuration entry %s.", value, name);
	}

	gp_widget_free(rootconfig);
	return (ret);
}


void Camera_Action::shot_to_file(GPParams* p, char* fn)
{
	int fd, retval;
	CameraFilePath camera_file_path;
	printf("Capturing.\n");

	/* NOP: This gets overridden in the library to /capt0000.jpg */
	strcpy(camera_file_path.folder, "/");
	strcpy(camera_file_path.name, "foo.jpg");

	retval = gp_camera_capture(p->camera, GP_CAPTURE_IMAGE, &camera_file_path, p->context);
	printf("  Retval: %d\n", retval);

	if (retval == GP_OK)
	{
		printf("Pathname on the camera: %s/%s\n", camera_file_path.folder, camera_file_path.name);

		CameraFile* file;

		fd = open(fn, O_CREAT | O_WRONLY, 0644);
		retval = gp_file_new_from_fd(&file, fd);
		printf("  Retval: %d\n", retval);
		retval = gp_camera_file_get(p->camera, camera_file_path.folder, camera_file_path.name, GP_FILE_TYPE_NORMAL, file, p->context);
		printf("  Retval: %d\n", retval);

		printf("Deleting.\n");
		retval = gp_camera_file_delete(p->camera, camera_file_path.folder, camera_file_path.name, p->context);
		printf("  Retval: %d\n", retval);

		gp_file_free(file);
	}



}

int Camera_Action::_find_widget_by_name(GPParams* p, const char* name, CameraWidget** child, CameraWidget** rootconfig) 
{
	int	ret;
	*rootconfig = NULL;

	ret = gp_camera_get_single_config(p->camera, name, child, p->context);
	if (ret == GP_OK) 
	{
		*rootconfig = *child;
		return GP_OK;
	}

	ret = gp_camera_get_config(p->camera, rootconfig, p->context);
	if (ret != GP_OK) return ret;
	ret = gp_widget_get_child_by_name(*rootconfig, name, child);
	if (ret != GP_OK)
		ret = gp_widget_get_child_by_label(*rootconfig, name, child);
	if (ret != GP_OK) 
	{
		char* part, * s, * newname;

		newname = strdup(name);
		if (!newname)
			return GP_ERROR_NO_MEMORY;

		*child = *rootconfig;
		part = newname;
		while (part[0] == '/')
			part++;
		while (1) 
		{
			CameraWidget* tmp;

			s = strchr(part, '/');
			if (s)
				*s = '\0';
			ret = gp_widget_get_child_by_name(*child, part, &tmp);
			if (ret != GP_OK)
				ret = gp_widget_get_child_by_label(*child, part, &tmp);
			if (ret != GP_OK)
				break;
			*child = tmp;
			if (!s) {
				/* end of path */
				free(newname);
				return GP_OK;
			}
			part = s + 1;
			while (part[0] == '/')
				part++;
		}
		gp_context_error(p->context, "%s not found in configuration tree.", newname);
		free(newname);
		gp_widget_free(*rootconfig);
		return GP_ERROR;
	}
	return GP_OK;
}

int Camera_Action::print_widget(GPParams* p, const char* name, CameraWidget* widget)
{
	const char* label;
	CameraWidgetType	type;
	int ret, readonly;

	ret = gp_widget_get_type(widget, &type);
	if (ret != GP_OK)
		return ret;
	ret = gp_widget_get_label(widget, &label);
	if (ret != GP_OK)
		return ret;

	ret = gp_widget_get_readonly(widget, &readonly);
	if (ret != GP_OK)
		return ret;

	printf("Label: %s\n", label); /* "Label:" is not i18ned, the "label" variable is */
	printf("Readonly: %d\n", readonly);
	switch (type) {
	case GP_WIDGET_TEXT: {		/* char *		*/
		char* txt;

		ret = gp_widget_get_value(widget, &txt);
		if (ret == GP_OK) {
			printf("Type: TEXT\n"); /* parsed by scripts, no i18n */
			printf("Current: %s\n", txt);
		}
		else {
			gp_context_error(p->context, "Failed to retrieve value of text widget %s.", name);
		}
		break;
	}
	case GP_WIDGET_RANGE: {	/* float		*/
		float	f, t, b, s;

		ret = gp_widget_get_range(widget, &b, &t, &s);
		if (ret == GP_OK)
			ret = gp_widget_get_value(widget, &f);
		if (ret == GP_OK) {
			printf("Type: RANGE\n");	/* parsed by scripts, no i18n */
			printf("Current: %g\n", f);	/* parsed by scripts, no i18n */
			printf("Bottom: %g\n", b);	/* parsed by scripts, no i18n */
			printf("Top: %g\n", t);	/* parsed by scripts, no i18n */
			printf("Step: %g\n", s);	/* parsed by scripts, no i18n */
		}
		else {
			gp_context_error(p->context, "Failed to retrieve values of range widget %s.", name);
		}
		break;
	}
	case GP_WIDGET_TOGGLE: {	/* int		*/
		int	t;

		ret = gp_widget_get_value(widget, &t);
		if (ret == GP_OK) {
			printf("Type: TOGGLE\n");
			printf("Current: %d\n", t);
		}
		else {
			gp_context_error(p->context, "Failed to retrieve values of toggle widget %s.", name);
		}
		break;
	}
	case GP_WIDGET_DATE: {		/* int			*/
// 		int	t;
// 		time_t	xtime;
// 		struct tm* xtm;
// 		char	timebuf[200];
// 
// 		ret = gp_widget_get_value(widget, &t);
// 		if (ret != GP_OK) {
// 			gp_context_error(p->context, "Failed to retrieve values of date/time widget %s.", name);
// 			break;
// 		}
// 		xtime = t;
// 		xtm = localtime(&xtime);
// 		ret = my_strftime(timebuf, sizeof(timebuf), "%c", xtm);
// 		printf("Type: DATE\n");
// 		printf("Current: %d\n", t);
// 		printf("Printable: %s\n", timebuf);
// 		printf("Help: %s\n", "Use 'now' as the current time when setting.\n");
		break;
	}
		case GP_WIDGET_MENU:
		case GP_WIDGET_RADIO: { /* char *		*/
			int cnt, i;
			char* current;

			ret = gp_widget_get_value(widget, &current);
			if (ret == GP_OK) {
				cnt = gp_widget_count_choices(widget);
				if (type == GP_WIDGET_MENU)
					printf("Type: MENU\n");
				else
					printf("Type: RADIO\n");
				printf("Current: %s\n", current);
				for (i = 0; i < cnt; i++) {
					const char* choice;
					ret = gp_widget_get_choice(widget, i, &choice);
					printf("Choice: %d %s\n", i, choice);
				}
			}
			else {
				gp_context_error(p->context, "Failed to retrieve values of radio widget %s.", name);
			}
			break;
		}

		/* ignore: */
		case GP_WIDGET_WINDOW:
		case GP_WIDGET_SECTION:
		case GP_WIDGET_BUTTON:
			break;
	}

	printf("END\n");
	return GP_OK;
}

int Camera_Action::get_widget(GPParams* p, const char* name, CameraWidget* widget)
{
	std::vector<std::string> choicelist;

	const char* label;
	CameraWidgetType	type;
	int ret, readonly;

	ret = gp_widget_get_type(widget, &type);
	if (ret != GP_OK)
		return ret;
	ret = gp_widget_get_label(widget, &label);
	if (ret != GP_OK)
		return ret;

	ret = gp_widget_get_readonly(widget, &readonly);
	if (ret != GP_OK)
		return ret;

	printf("Label: %s\n", label); /* "Label:" is not i18ned, the "label" variable is */
	printf("Readonly: %d\n", readonly);
	switch (type) 
	{
		case GP_WIDGET_TEXT: /* char *		*/
		{		
			char* txt;

			ret = gp_widget_get_value(widget, &txt);
			if (ret == GP_OK) 
			{
				printf("Type: TEXT\n"); /* parsed by scripts, no i18n */
				printf("Current: %s\n", txt);
			}
			else 
			{
				gp_context_error(p->context, "Failed to retrieve value of text widget %s.", name);
			}
			break;
		}

		case GP_WIDGET_RANGE: /* float		*/
		{	
			float	f, t, b, s;
			ret = gp_widget_get_range(widget, &b, &t, &s);

			if (ret == GP_OK)
				ret = gp_widget_get_value(widget, &f);

			if (ret == GP_OK) 
			{
				printf("Type: RANGE\n");	/* parsed by scripts, no i18n */
				printf("Current: %g\n", f);	/* parsed by scripts, no i18n */
				printf("Bottom: %g\n", b);	/* parsed by scripts, no i18n */
				printf("Top: %g\n", t);	/* parsed by scripts, no i18n */
				printf("Step: %g\n", s);	/* parsed by scripts, no i18n */
			}
			else 
			{
				gp_context_error(p->context, "Failed to retrieve values of range widget %s.", name);
			}
			break;
		}

		case GP_WIDGET_TOGGLE: /* int		*/
		{	
			int	t;
			ret = gp_widget_get_value(widget, &t);

			if (ret == GP_OK) 
			{
				printf("Type: TOGGLE\n");
				printf("Current: %d\n", t);
			}
			else 
			{
				gp_context_error(p->context, "Failed to retrieve values of toggle widget %s.", name);
			}
			break;
		}

		case GP_WIDGET_DATE: 		/* int			*/
		{
	// 		int	t;
	// 		time_t	xtime;
	// 		struct tm* xtm;
	// 		char	timebuf[200];
	// 
	// 		ret = gp_widget_get_value(widget, &t);
	// 		if (ret != GP_OK) {
	// 			gp_context_error(p->context, "Failed to retrieve values of date/time widget %s.", name);
	// 			break;
	// 		}
	// 		xtime = t;
	// 		xtm = localtime(&xtime);
	// 		ret = my_strftime(timebuf, sizeof(timebuf), "%c", xtm);
	// 		printf("Type: DATE\n");
	// 		printf("Current: %d\n", t);
	// 		printf("Printable: %s\n", timebuf);
	// 		printf("Help: %s\n", "Use 'now' as the current time when setting.\n");
			break;
		}
		case GP_WIDGET_MENU:
		case GP_WIDGET_RADIO: /* char *		*/
		{ 
			int cnt, i;
			char* current;

			ret = gp_widget_get_value(widget, &current);
			if (ret == GP_OK) 
			{
				cnt = gp_widget_count_choices(widget);
				if (type == GP_WIDGET_MENU)
					printf("Type: MENU\n");
				else
					printf("Type: RADIO\n");
				printf("Current: %s\n", current);

				for (i = 0; i < cnt; i++) 
				{
					const char* choice;
					ret = gp_widget_get_choice(widget, i, &choice);
					printf("Choice: %d %s\n", i, choice);

					choicelist.push_back(std::string(choice));
				}
			}
			else 
			{
				gp_context_error(p->context, "Failed to retrieve values of radio widget %s.", name);
			}
			break;
		}
	}

	//////////////////////////////////////////////////////////////////////////


	if (std::string(name) == "iso" && choicelist.empty() == false )
	{
		
	}

	return GP_OK;
}


int Camera_Action::action_camera_wait_event(GPParams* p, const char* arg)
{
	int ret;
	struct waitparams wp;
	CameraEventType	event;
	void* data = NULL;
	CameraFilePath* fn;
	CameraFilePath last;
	struct timeval	xtime;
	int events, frames;

	gettimeofday(&xtime, NULL);
	memset(&last, 0, sizeof(last));

	wp.type = WAIT_EVENTS;
	wp.u.events = 1000000;
	if (!arg || !strlen(arg)) 
	{
		printf("Waiting for events from camera. Press Ctrl-C to abort.\n");
	}
	else 
	{
		int x;
		if ((arg[strlen(arg) - 1] == 'f') && sscanf(arg, "%df", &x)) { /* exact nr of frames */
			wp.type = WAIT_FRAMES;
			wp.u.frames = x;
			printf("Waiting for %d frames from the camera. Press Ctrl-C to abort.\n", x);
		}
		else
			if ((strlen(arg) > 2) && (!strcmp(&arg[strlen(arg) - 2], "ms")) && sscanf(arg, "%dms", &x)) { /* exact milliseconds */
				wp.type = WAIT_TIME;
				wp.u.milliseconds = x;
				printf("Waiting for %d milliseconds for events from camera. Press Ctrl-C to abort.\n", x);
			}
			else
				if ((wp.type != WAIT_TIME) && (arg[strlen(arg) - 1] == 's') && sscanf(arg, "%ds", &x)) { /* exact seconds */
					wp.type = WAIT_TIME;
					wp.u.milliseconds = x * 1000;
					printf("Waiting for %d seconds for events from camera. Press Ctrl-C to abort.\n", x);
				}
				else if ((wp.type == WAIT_EVENTS) && sscanf(arg, "%d", &x)) {
					wp.u.events = atoi(arg);
					printf("Waiting for %d events from camera. Press Ctrl-C to abort.\n", wp.u.events);
				}
				else {
					wp.type = WAIT_STRING;
					wp.u.str = (char*)arg;
					printf("Waiting for '%s' event from camera. Press Ctrl-C to abort.\n", wp.u.str);
				}
	}

	events = frames = 0;
	while (1) 
	{
		int leftoverms = 10000;
		struct timeval	ytime;
		int		x, exitloop;

		//if (glob_cancel) break;

		exitloop = 0;
		switch (wp.type) 
		{
			case WAIT_STRING:
				break;
			case WAIT_EVENTS:
				if (events >= wp.u.events) exitloop = 1;
				break;
			case WAIT_FRAMES:
				if (frames >= wp.u.frames) exitloop = 1;
				break;
			case WAIT_TIME:
				gettimeofday(&ytime, NULL);

				x = ((ytime.tv_usec - xtime.tv_usec) + (ytime.tv_sec - xtime.tv_sec) * 1000000) / 1000;
				if (x >= wp.u.milliseconds) { exitloop = 1; break; }

				// if left over time is < 1s, set it... otherwise wait at most 1s
				if ((wp.u.milliseconds - x) < leftoverms)
					leftoverms = wp.u.milliseconds - x;
		}
		if (exitloop) break;

		data = NULL;
		ret = gp_camera_wait_for_event(p->camera, leftoverms, &event, &data, p->context);
		if (ret != GP_OK)
			return ret;
		events++;

		switch (event) 
		{
		case GP_EVENT_UNKNOWN:
			if (data) 
			{
				printf("UNKNOWN %s\n", (char*)data);
				if (wp.type == WAIT_STRING) 
				{
					if ( strstr((const char*)data, wp.u.str) ) 
					{
						printf("event found, stopping wait!\n");
						return GP_OK;
					}
				}
				else if (wp.type == WAIT_EVENTS)
				{
					printf("event !\n");
				}
				else if (wp.type == WAIT_TIME)
				{
					printf("wait time!\n");
				}
				else
					printf("event ????? \n");
			}
			else 
			{
				printf("UNKNOWN\n");
			}
			break;

		case GP_EVENT_TIMEOUT:
			printf("TIMEOUT\n");
			//exitloop = 1;
			break;

		case GP_EVENT_CAPTURE_COMPLETE:
			printf("CAPTURECOMPLETE\n");
			if ((wp.type == WAIT_STRING) && strstr("CAPTURECOMPLETE", wp.u.str)) 
			{
				printf("event found, stopping wait!\n");
				return GP_OK;
			}
			break;
/*
		case GP_EVENT_FILE_ADDED:
			frames++;

			fn = (CameraFilePath*)data;

			if ((downloadtype == DT_NO_DOWNLOAD) || (p->flags & FLAGS_KEEP_RAW) && (!strstr(fn->name, ".jpg") && !strstr(fn->name, ".JPG")))) 
			{
				printf("FILEADDED %s %s\n", fn->name, fn->folder);
				if ((wp.type == WAIT_STRING) && strstr("FILEADDED", wp.u.str)) 
				{
					printf("event found, stopping wait!\n");
					return GP_OK;
				}
				continue;
			}

			/ * Otherwise download the image and continue... * /
			if (strcmp(fn->folder, last.folder)) {
				strcpy(last.folder, fn->folder);
				ret = set_folder_action(p, fn->folder);
				if (ret != GP_OK) {
					cli_error_print("Could not set folder.");
					return (ret);
				}
			}
			ret = get_file_common(fn->name, GP_FILE_TYPE_NORMAL);
			if (ret != GP_OK) {
				cli_error_print("Could not get image.");
				if (ret == GP_ERROR_FILE_NOT_FOUND) {
					/ * Buggy libcanon.so?
					* Can happen if this was the first capture after a
					* CF card format, or during a directory roll-over,
					* ie: CANON100 -> CANON101
					* /
					cli_error_print("Buggy libcanon.so?");
				}
				return (ret);
			}

			if (!(p->flags & FLAGS_KEEP)) {
				do {
					ret = delete_file_action(p, p->folder, fn->name);
				} while (ret == GP_ERROR_CAMERA_BUSY);
				if (ret != GP_OK) {
					cli_error_print("Could not delete image.");
					/ * dont continue in event loop * /
				}
			}
			if ((wp.type == WAIT_STRING) && strstr("FILEADDED", wp.u.str)) {
				printf("event found, stopping wait!\n");
				return GP_OK;
			}
			break;
		case GP_EVENT_FOLDER_ADDED:
			fn = (CameraFilePath*)data;
			printf("FOLDERADDED %s %s\n", fn->name, fn->folder);
			if ((wp.type == WAIT_STRING) && strstr("FOLDERADDED", wp.u.str)) {
				printf("event found, stopping wait!\n");
				return GP_OK;
			}
			break;
		case GP_EVENT_FILE_CHANGED:
			fn = (CameraFilePath*)data;
			printf("FILECHANGED %s %s\n", fn->name, fn->folder);
			if ((wp.type == WAIT_STRING) && strstr("FILECHANGED", wp.u.str)) {
				printf("event found, stopping wait!\n");
				return GP_OK;
			}
			break;*/
		}
		free(data);
	}
	return GP_OK;
}



int Camera_Action::action_camera_wait_focus(GPParams* p)
{
	int ret;
	struct waitparams wp;
	CameraEventType	event;
	void* data = NULL;

	wp.type = WAIT_EVENTS;
	wp.u.events = 1000000;

	bool loop = true;

	while (loop)
	{
		int leftoverms = 10000;
		struct timeval	ytime;
		int		x, exitloop;

		data = NULL;
		ret = gp_camera_wait_for_event(p->camera, leftoverms, &event, &data, p->context);

		if (ret != GP_OK)
			return ret;

		switch (event)
		{
			case GP_EVENT_UNKNOWN:
				if (data)
				{
					printf("UNKNOWN %s\n", (char*)data);
					if (strstr((char*)data, "CTGInfoCheckComplete") != NULL)
					{
						loop = false;
					}
				}
				else
				{
					printf("UNKNOWN\n");
				}
				break;

			case GP_EVENT_TIMEOUT:
				printf("TIMEOUT\n");
				free(data);
				return GP_ERROR;
				break;
		}

		free(data);
	}
	return GP_OK;
}