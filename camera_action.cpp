
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>

#include "./camera_action.h"

cameraaction::cameraaction()
{
}
cameraaction::~cameraaction()
{
}

void cameraaction::shot_to_file(Camera* camera, GPContext* context, char* fn)
{
	int fd, retval;
	CameraFile* file;
	CameraFilePath camera_file_path;

	printf("Capturing.\n");

	/* NOP: This gets overridden in the library to /capt0000.jpg */
	strcpy(camera_file_path.folder, "/");
	strcpy(camera_file_path.name, "foo.jpg");

	retval = gp_camera_capture(camera, GP_CAPTURE_IMAGE, &camera_file_path, context);
	printf("  Retval: %d\n", retval);

	printf("Pathname on the camera: %s/%s\n", camera_file_path.folder, camera_file_path.name);

	fd = open(fn, O_CREAT | O_WRONLY, 0644);
	retval = gp_file_new_from_fd(&file, fd);
	printf("  Retval: %d\n", retval);
	retval = gp_camera_file_get(camera, camera_file_path.folder, camera_file_path.name, GP_FILE_TYPE_NORMAL, file, context);
	printf("  Retval: %d\n", retval);

	printf("Deleting.\n");
	retval = gp_camera_file_delete(camera, camera_file_path.folder, camera_file_path.name, context);
	printf("  Retval: %d\n", retval);

	gp_file_free(file);
}