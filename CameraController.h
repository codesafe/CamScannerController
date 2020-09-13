#ifndef __CameraControllerApi__
#define __CameraControllerApi__

#include <iostream>
#include <string>
#include <vector>
#include <exception>
#include <gphoto2/gphoto2-camera.h>

#include "predef.h"

using std::string;

class CameraController
{    
    static GPContextErrorFunc _error_callback(GPContext *context, const char *text, void *data);
    static GPContextMessageFunc _message_callback(GPContext *context, const char *text, void *data);
        
    public:
		CameraController();
		~CameraController();
		void init();
		void release();

		void setPort(string port);
		bool camera_found();
		bool is_initialized();

		bool is_busy();
		void is_bussy(bool busy);
		int capture(const char *filename);

		int get_settings_value(const char *key, string &val);
		int get_settings_choices(const char *key, std::vector<string> &choices);
		int set_settings_value(const char *key, const char *val);
		int set_settings_value(const char* key, int val);

		void apply_essential_param_param();
		void set_essential_param(CAMERA_PARAM param, string value);

//         int preview(const char **file_data);
//         int bulb(const char *filename, string &data);
//         int get_settings(ptree &sett);
//         int get_files(ptree &tree);
//         int get_file(const char *filename, const char *filepath, string &base64out);
                
    private:

        Camera *_camera;
        GPContext *_context;

        bool _is_busy;
        bool _liveview_running;
        bool _camera_found;
        bool _is_initialized;
        bool _save_images;

		string iso;
		string shutterspeed;
		string aperture;

        void _init_camera();
        int _wait_and_handle_event (useconds_t waittime, CameraEventType *type, int download);
		void _set_capturetarget(int index);

//         int _get_files(ptree &tree, const char *folder);
//         void _build_settings_tree(CameraWidget *w);
//         void _read_widget(CameraWidget *w, ptree &tree, string node);
//         void _get_item_value(CameraWidget *w, ptree &tree);

};


#endif 
