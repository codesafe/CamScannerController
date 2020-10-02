#include "CameraController.h"

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string>
#include <pthread.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <string.h>
#include <utime.h>

#include "utils.h"

using namespace std;


void CameraController::release()
{
	gp_camera_exit(_camera, _context);
}

CameraController::CameraController()
{
	_camera_found = false;
	_is_busy = false;
	_is_initialized = false;
	_liveview_running = false;
	_camera = NULL;
	_context = NULL;
	_save_images = true;
	_halfpressed = false;

// 	iso = ISO_VALUE;
// 	shutterspeed = SHUTTERSPEED_VALUE;
// 	aperture = APERTURE_VALUE;

	iso = "1600";
	shutterspeed = "1/60";
	aperture = "10";
}

void CameraController::init()
{
    if(!_camera_found)
	{
         _init_camera();

        if(_save_images && _camera_found)
            _set_capturetarget(CAPTURE_TO_RAM);
    }
}

string CameraController::getport()
{
	return port;
}

void CameraController::setPort(string port)
{
	GPPortInfoList* portinfo_list;
	char verified_port[64] = { 0, };
	strcpy(verified_port, port.c_str());
	this->port = port;


	GPPortInfo portinfo;
	if (gp_port_info_list_new(&portinfo_list) < GP_OK)
	{
		Logger::log(-1, "Error gp_port_info_list_new");
		return;
	}

	int result = gp_port_info_list_load(portinfo_list);
	if (result < 0)
	{
		Logger::log(-1, "Error gp_port_info_list_load");
		gp_port_info_list_free(portinfo_list);
		return;
	}

	int p = gp_port_info_list_lookup_path(portinfo_list, verified_port);
	if (p < GP_OK)
	{
		Logger::log(-1, "Error gp_port_info_list_lookup_path : %d", p);
		return;
	}

	int r = gp_port_info_list_get_info(portinfo_list, p, &portinfo);
	if (r < GP_OK)
	{
		Logger::log(-1, "Error gp_port_info_list_get_info : %d", r);
		return;
	}

	gp_camera_set_port_info(_camera, portinfo);

	gp_port_info_list_free(portinfo_list);
}

void CameraController::_init_camera()
{
    gp_camera_new(&_camera);
    _context = gp_context_new();
    gp_context_set_error_func(_context, (GPContextErrorFunc)CameraController::_error_callback, NULL);
    gp_context_set_message_func(_context, (GPContextMessageFunc)CameraController::_message_callback, NULL);

    int ret = gp_camera_init(_camera, _context);
    if(ret < GP_OK)
	{
        gp_camera_free(_camera);
    } 
	else 
	{
        _camera_found = true;
    }
    _is_initialized = true;
}

CameraController::~CameraController()
{
    gp_camera_exit(_camera, _context);
    gp_context_unref(_context);
}


bool CameraController::is_busy()
{
    return _is_busy;
}

void CameraController::is_bussy(bool busy)
{
    _is_busy = busy;
}

bool CameraController::camera_found()
{
    return _camera_found;
}

bool CameraController::is_initialized()
{
    return _is_initialized;
}

int CameraController::capture(const char *filename)
{
    _is_busy = true;

    int ret, fd ;
    CameraFile *file;
    CameraFilePath path;

    strcpy(path.folder, "/");
	strcpy(path.name, filename);

	ret = gp_camera_capture(_camera, GP_CAPTURE_IMAGE, &path, _context);
    if (ret != GP_OK)
        return ret;


    if(_save_images == false)
	{
        ret = gp_file_new(&file);
    } else {
        fd  = open(filename, O_CREAT | O_RDWR, 0644);
        ret = gp_file_new_from_fd(&file, fd);
    }

    if (ret != GP_OK){
        _is_busy = false;
        return ret;
    }

	ret = gp_camera_file_get(_camera, path.folder, path.name, GP_FILE_TYPE_NORMAL, file, _context);

    if (ret != GP_OK){
        _is_busy = false;
        return ret;
    }

    if(_save_images == false)
	{
        ret = gp_camera_file_delete(_camera, path.folder, path. name, _context);

        if (ret != GP_OK){
            _is_busy = false;
            return ret;
        }
    }


    int waittime = 10;
    CameraEventType type;
    void *eventdata;

    while(1) 
	{
        gp_camera_wait_for_event(_camera, waittime, &type, &eventdata, _context);

        if(type == GP_EVENT_TIMEOUT) {
            break;
        }
        else if (type == GP_EVENT_CAPTURE_COMPLETE || type == GP_EVENT_FILE_ADDED) {
            waittime = 10;
        }
        else if (type != GP_EVENT_UNKNOWN) {
            printf("Unexpected event received from camera: %d\n", (int)type);
        }
    }
    
    gp_file_free(file);


    _is_busy = false;
    return true;
}

int CameraController::capture2(const char* filename)
{
	_is_busy = true;

	int ret, fd;
	CameraFile* file;
	CameraFilePath path;

	strcpy(path.folder, "/");
	strcpy(path.name, filename);

	ret = gp_camera_capture(_camera, GP_CAPTURE_IMAGE, &path, _context);
	if (ret != GP_OK)
		return ret;

	_is_busy = false;
	return true;
}

int CameraController::capture3(const char* filename)
{
	_is_busy = true;

	set_settings_value("eosremoterelease", "Immediate");

	void* data = NULL;
	CameraEventType	event;
	CameraFilePath* fn;
	int ret;
	bool loop = true;

	while (loop)
	{
		int leftoverms = 1000;
		ret = gp_camera_wait_for_event(_camera, leftoverms, &event, &data, _context);
		if (ret != GP_OK)
			return ret;

		switch (event) 
		{
			case GP_EVENT_FILE_ADDED:
			{
				fn = (CameraFilePath*)data;

				CameraFile* file;
				CameraFileInfo info;
				ret = gp_camera_file_get_info(_camera, fn->folder, fn->name, &info, _context);
				if (ret != GP_OK)
				{
					printf("gp_camera_file_get_info %s : %d Error\n", filename, ret);

					free(data);
					_is_busy = false;
					return ret;
				}
				else
				{
					printf("gp_camera_file_get_info %s : %s success\n", fn->folder, fn->name);
				}

				int fd;
				fd = open(filename, O_CREAT | O_RDWR, 0644);
				ret = gp_file_new_from_fd(&file, fd);
				if (ret != GP_OK)
				{
					printf("gp_file_new_from_fd %s Error\n", filename);
					gp_file_unref(file);
					free(data);
					_is_busy = false;
					return ret;
				}

				ret = gp_camera_file_get(_camera, fn->folder, fn->name, GP_FILE_TYPE_NORMAL, file, _context);
				if (ret != GP_OK)
				{
					printf("gp_camera_file_get %s : %d Error\n", filename, ret);
					gp_file_unref(file);
					free(data);
					_is_busy = false;
					return ret;
				}
				else
				{
					printf("gp_camera_file_get %s : %s success\n", fn->folder, fn->name);
				}

				//gp_file_unref(file);
				gp_file_free(file);
				//loop = false;
			}
			break;

			case GP_EVENT_CAPTURE_COMPLETE:
			{
				loop = false;
				printf("Capture %s Done!\n", filename);
			}
			break;
		}
		free(data);
	}

	set_settings_value("eosremoterelease", "Release Full");
	_is_busy = false;
	_halfpressed = false;

	return true;
}

int CameraController::downloadimage(const char* filename)
{
	_is_busy = true;

	int ret, fd;
	CameraFile* file;
	CameraFilePath path;

	strcpy(path.folder, "/");
	strcpy(path.name, filename);

/*
	ret = gp_camera_capture(_camera, GP_CAPTURE_IMAGE, &path, _context);
	if (ret != GP_OK)
		return ret;
*/


	if (_save_images == false)
	{
		ret = gp_file_new(&file);
	}
	else 
	{
		fd = open(filename, O_CREAT | O_RDWR, 0644);
		ret = gp_file_new_from_fd(&file, fd);
	}

	if (ret != GP_OK) {
		_is_busy = false;
		return ret;
	}

	ret = gp_camera_file_get(_camera, path.folder, path.name, GP_FILE_TYPE_NORMAL, file, _context);

	if (ret != GP_OK) {
		_is_busy = false;
		return ret;
	}

	if (_save_images == false)
	{
		ret = gp_camera_file_delete(_camera, path.folder, path.name, _context);

		if (ret != GP_OK) {
			_is_busy = false;
			return ret;
		}
	}


	int waittime = 10;
	CameraEventType type;
	void* eventdata;

	while (1)
	{
		gp_camera_wait_for_event(_camera, waittime, &type, &eventdata, _context);

		if (type == GP_EVENT_TIMEOUT) {
			break;
		}
		else if (type == GP_EVENT_CAPTURE_COMPLETE || type == GP_EVENT_FILE_ADDED) {
			waittime = 10;
		}
		else if (type != GP_EVENT_UNKNOWN) {
			printf("Unexpected event received from camera: %d\n", (int)type);
		}
	}

	gp_file_free(file);

	_is_busy = false;
}


int CameraController::get_settings_choices(const char* key, vector<string>& choices)
{
	_is_busy = true;
	CameraWidget* w, * child;
	int ret;

	ret = gp_camera_get_config(_camera, &w, _context);
	if (ret < GP_OK) {
		_is_busy = false;
		return ret;
	}

	ret = gp_widget_get_child_by_name(w, key, &child);
	if (ret < GP_OK)
	{
		_is_busy = false;
		return ret;
	}

	int count = gp_widget_count_choices(child);
	for (int i = 0; i < count; i++) {
		const char* choice;
		ret = gp_widget_get_choice(child, i, &choice);

		if (ret < GP_OK)
		{
			_is_busy = false;
			return ret;
		}

		choices.push_back(string(choice));
	}
	_is_busy = false;
	return true;
}

int CameraController::get_settings_value(const char* key, string& val)
{
	_is_busy = true;
	CameraWidget* w, * child;
	int ret;

	ret = gp_camera_get_config(_camera, &w, _context);
	if (ret < GP_OK)
	{
		_is_busy = false;
		return ret;
	}

	ret = gp_widget_get_child_by_name(w, key, &child);
	if (ret < GP_OK)
	{
		_is_busy = false;
		return ret;
	}

	void* item_value;
	ret = gp_widget_get_value(child, &item_value);
	if (ret < GP_OK)
	{
		_is_busy = false;
		return ret;
	}

	unsigned char* value = static_cast<unsigned char*>(item_value);
	val.append((char*)value);

	_is_busy = false;
	return true;
}

int CameraController::set_settings_value(const char* key, const char* val)
{
	if (key == std::string("eosremoterelease") && val == std::string("Press Half"))
		_halfpressed = true;
	else if (key == std::string("eosremoterelease") && val == std::string("Release Full"))
		_halfpressed = false;

	_is_busy = true;
	CameraWidget* w, * child;
	int ret = gp_camera_get_config(_camera, &w, _context);
	if (ret < GP_OK)
	{
		_is_busy = false;
		return ret;
	}

	ret = gp_widget_get_child_by_name(w, key, &child);
	if (ret < GP_OK)
	{
		_is_busy = false;
		return ret;
	}

	ret = gp_widget_set_value(child, val);
	if (ret < GP_OK)
	{
		_is_busy = false;
		return ret;
	}


	ret = gp_camera_set_config(_camera, w, _context);
	if (ret < GP_OK)
	{
		_is_busy = false;
		return ret;
	}
	gp_widget_free(w);

	_is_busy = false;
	return ret;
}

int CameraController::set_settings_value(const char* key, int val)
{
	_is_busy = true;
	CameraWidget* w, * child;
	int ret = gp_camera_get_config(_camera, &w, _context);
	if (ret < GP_OK)
	{
		_is_busy = false;
		return ret;
	}

	ret = gp_widget_get_child_by_name(w, key, &child);
	if (ret < GP_OK)
	{
		_is_busy = false;
		return ret;
	}

	ret = gp_widget_set_value(child, &val);
	if (ret != GP_OK)
	{
		_is_busy = false;
		return ret;
	}


	ret = gp_camera_set_config(_camera, w, _context);
	if (ret < GP_OK)
	{
		_is_busy = false;
		return ret;
	}

	gp_widget_free(w);

	_is_busy = false;
	return ret;
}

/**
 borrowed gphoto2
 http://sourceforge.net/p/gphoto/code/HEAD/tree/trunk/gphoto2/gphoto2/main.c#l834
 */
int CameraController::_wait_and_handle_event(useconds_t waittime, CameraEventType* type, int download)
{
	int 		result;
	CameraEventType	evtype;
	void* data;
	CameraFilePath* path;

	if (!type) type = &evtype;
	evtype = GP_EVENT_UNKNOWN;
	data = NULL;
	result = gp_camera_wait_for_event(_camera, waittime, type, &data, _context);
	if (result == GP_ERROR_NOT_SUPPORTED)
	{
		*type = GP_EVENT_TIMEOUT;
		//usleep(waittime*1000);
		Utils::Sleep(waittime * 1000);
		return GP_OK;
	}

	if (result != GP_OK)
		return result;
	path = (CameraFilePath*)data;
	switch (*type) {
	case GP_EVENT_TIMEOUT:
		break;
	case GP_EVENT_CAPTURE_COMPLETE:
		break;
	case GP_EVENT_FOLDER_ADDED:
		free(data);
		break;
	case GP_EVENT_FILE_ADDED:
		//result = save_captured_file (path, download);
		free(data);
		/* result will fall through to final return */
		break;
	case GP_EVENT_UNKNOWN:
		free(data);
		break;
	default:
		break;
	}
	return result;
}

void CameraController::_set_capturetarget(int index)
{
	std::vector<string> choices;

	int ret;
	ret = get_settings_choices("capturetarget", choices);

	for (int i = 0; i < choices.size(); i++)
	{
		printf("Store type : %s\n", choices[i].c_str());
	}

	if (ret && index < choices.size())
	{
		string choice = choices.at(index);
		set_settings_value("capturetarget", choice.c_str());
	}

}

GPContextErrorFunc CameraController::_error_callback(GPContext* context, const char* text, void* data) {

	return 0;
}

GPContextMessageFunc CameraController::_message_callback(GPContext* context, const char* text, void* data) {

	return 0;
}

int CameraController::apply_autofocus(int camnum, bool enable)
{
	int ret = set_settings_value("cancelautofocus", enable ? "0" : "1");
	if (ret < GP_OK)
	{
		Logger::log(camnum, "Error cancelautofocus : %d",ret);
		return ret;
	}

	ret = set_settings_value("autofocusdrive", enable ? "True" : "False");
	if (ret < GP_OK)
	{
		Logger::log(camnum, "Error autofocusdrive : %d", ret);
		return ret;
	}


	return ret;
}

// 아래의 param 셋팅
int CameraController::apply_essential_param_param(int camnum)
{
	int ret = 0;
// 	ret = set_settings_value("autofocusdrive", "0");
// 	if (ret < GP_OK)
// 	{
// 		Logger::log(0, "Error autofocusdrive : %d : ret", camnum, ret);
// 		return ret;
// 	}

/*
	ret = set_settings_value("cancelautofocus", "1");
	if (ret < GP_OK)
	{
		Logger::log(0, "Error cancelautofocus : %d : ret", camnum, ret);
		return ret;
	}*/

	ret = set_settings_value("iso", iso.c_str());					// "400"
	if (ret < GP_OK)
	{
		Logger::log(camnum, "Error set_settings_value iso : %s : %d : %d", iso.c_str(), camnum, ret);
		return ret;
	}

	ret = set_settings_value("aperture", aperture.c_str());				// "10"
	if (ret < GP_OK)
	{
		Logger::log(camnum, "Error set_settings_value aperture : %s : %d : %d", aperture.c_str(), camnum, ret);
		return ret;
	}

	ret = set_settings_value("shutterspeed", shutterspeed.c_str());		// "1/100"
	if (ret < GP_OK)
	{
		Logger::log(camnum, "Error set_settings_value shutterspeed : %s : %d : %d", shutterspeed.c_str(), camnum, ret);
		return ret;
	}

	return ret;
}

void CameraController::set_essential_param(CAMERA_PARAM param, string value)
{
	switch (param)
	{
	case ISO:
		iso = value;
		break;
	case SHUTTERSPEED:
		shutterspeed = value;
		break;
	case APERTURE:
		aperture = value;
		break;
	}
}

/*

int CameraController::preview(const char **file_data)
{
	if(this->_camera_found == false)
		return GP_ERROR;

    this->_is_busy = true;
    int ret;
    CameraFile *file;
    ret = gp_file_new(&file);
    if (ret != GP_OK){
        this->_is_busy = false;
        return ret;
    }

	ret = gp_camera_capture_preview(this->_camera, file, this->_ctx);

    if(ret != GP_OK){
        this->_is_busy = false;
        return ret;
    }

    unsigned long int file_size = 0;
    const char *data;
	ret = gp_file_get_data_and_size(file, &data, &file_size);

    if(ret != GP_OK){
        this->_is_busy = false;
        return ret;
    }
    this->_is_busy = false;
    *file_data = new char[file_size];
    memcpy((void *)*file_data, data, file_size);
    gp_file_unref(file);
    
    return static_cast<int>(file_size);
}
*/

/*

int CameraController::bulb(const char *filename, string &data){
    / *
    This doesn't work with a nikon and i've no canon available

    int ret;
    CameraEventType evtype;
    std::vector<string> choices;

    ret = this->get_settings_choices("shutterspeed2", choices);
    size_t count = choices.size();
    if(ret && count > 0){
        string choice = choices.at(count - 1);
        this->set_settings_value("shutterspeed2", choice.c_str());
    }

    ret = this->set_settings_value("d100", "-1");

    CameraFile *file;
    CameraFilePath path;

    strcpy(path.folder, "/");
	strcpy(path.name, filename);

	ret = gp_camera_capture(this->_camera, GP_CAPTURE_IMAGE, &path, this->_ctx);
    if (ret != GP_OK)
        return ret;

    time_t lastsec = time(NULL)-3;
    struct timeval	tval;
    while(1){
        ret = this->_wait_and_handle_event(5, &evtype, 1);
    }

    if(ret != GP_OK)
        return ret;
    * /
    return true;
}
*/

/*

int CameraController::get_file(const char *filename, const char *filepath, string &base64out){
    this->_is_busy = true;
    int ret;
    CameraFile *file;

    ret = gp_file_new(&file);
    if(ret < GP_OK){
        this->_is_busy = false;
        return ret;
    }

    ret = gp_camera_file_get(this->_camera, filepath, filename, GP_FILE_TYPE_NORMAL, file, this->_ctx);
    if(ret < GP_OK){
        this->_is_busy = false;
        return ret;
    }

    this->_file_to_base64(file, base64out);
    this->_is_busy = false;
    return true;
}
*/

/*

int CameraController::get_files(ptree &tree)
{
    this->_is_busy = true;
    int ret = this->_get_files(tree, "/");
    this->_is_busy = false;
    return ret;
}
*/

/*

int CameraController::_get_files(ptree &tree, const char *path){
    int ret;

    const char	*name;
    CameraList *folder, *files;

    ret = gp_list_new(&folder);
    if(ret < GP_OK)
        return ret;

    ret = gp_camera_folder_list_folders(this->_camera, path,  folder, this->_ctx);
    if(ret < GP_OK)
        return ret;

    int count_folders = gp_list_count(folder);

    if(count_folders){
        for(int i = 0; i < count_folders; i++){
            gp_list_get_name(folder, i, &name);

            string abspath(path);
            abspath.append(name);
            abspath.append("/");

            this->_get_files(tree, abspath.c_str());
        }
    } else {
        bool show_thumb = Settings::get_value("general.thumbnail").compare("true") == 0;

        ret = gp_list_new(&files);
        if(ret < GP_OK)
            return ret;

        ret = gp_camera_folder_list_files(this->_camera, path, files, this->_ctx);
        if(ret < GP_OK)
            return ret;

        int count_files = gp_list_count(files);

        ptree current_folder, filelist;
        current_folder.put("absolute_path", path);


        unsigned long int file_size = 0;
        const char *file_data = NULL;

        for(int j = 0; j < count_files; j++){
            gp_list_get_name(files, j, &name);

            ptree valuechild;
            valuechild.put("name", name);

            std::string ext = std::string(name);
            boost::algorithm::to_lower(ext);
            ext = ext.substr(ext.find_last_of(".") + 1);
            if(show_thumb && ext.compare("avi") != 0){
                CameraFilePath cPath;
                strcpy(cPath.folder, path);
                strcpy(cPath.name, name);

                printf("start read file %s", name);

                CameraFile *file;
                ret = gp_file_new(&file);

                if (ret != GP_OK)
                    continue;

                ret = gp_camera_file_get(this->_camera, cPath.folder, cPath.name, GP_FILE_TYPE_NORMAL, file, this->_ctx);

                if (ret != GP_OK)
                    continue;

                ret = gp_file_get_data_and_size (file, &file_data, &file_size);
                if (ret != GP_OK)
                    continue;

                string thumb = "";
                if (Helper::get_thumbnail_from_exif(file_data, file_size, thumb) == false) {
                    //if we have no thumbnail data, we sending the whole image
                    std::string img(file_data, file_size);
                    Helper::to_base64(&img, thumb);
                }
                valuechild.put("thumbnail", thumb);

                gp_file_free(file);
                file_size = 0;
                file_data = NULL;
            }

            filelist.push_back(std::make_pair("", valuechild));
        }
        current_folder.put_child("files", filelist);
        tree.put_child("folder", current_folder);
        gp_list_free(files);

    }
    gp_list_free(folder);

    return true;
}*/


/*
int CameraController::get_settings(ptree &sett)
{
    this->_is_busy = true;

    CameraWidget *w, *children;
    int ret;
    ret = gp_camera_get_config(this->_camera, &w, this->_ctx);
    if(ret < GP_OK){
        this->_is_busy = false;
        return false;
    }

    ret = gp_widget_get_child_by_name(w, "main", &children);
    if(ret < GP_OK){
        this->_is_busy = false;
        return false;
    }

    this->_read_widget(children, sett, "settings");
    this->_is_busy = false;
    return true;
}*/


/*
void CameraController::_read_widget(CameraWidget *w,  ptree &tree, string node)
{
    const char  *name;
    ptree subtree;
    gp_widget_get_name(w, &name);
    string nodename = node + "." + name;

    int items = gp_widget_count_children(w);
    if(items > 0){
        for(int i = 0; i < items; i++){
            CameraWidget *item_widget;
            gp_widget_get_child(w, i, &item_widget);

            this->_read_widget(item_widget, subtree, nodename);
        }
    } else {
        this->_get_item_value(w, subtree);
    }

    tree.put_child(name, subtree);
}
*/


/*
void CameraController::_get_item_value(CameraWidget *w, ptree &tree)
{
    CameraWidgetType type;
    int id;
    const char *item_label, *item_name;
    ptree item_choices;
    void* item_value;
    gp_widget_get_id(w, &id);
    gp_widget_get_label(w, &item_label);
    gp_widget_get_name(w, &item_name);
    gp_widget_get_value(w, &item_value);
    gp_widget_get_type(w, &type);
    
    if(item_value == NULL)
        return;

    int number_of_choices = gp_widget_count_choices(w);
    if(number_of_choices > 0)
	{
        ptree choice_value;
        for(int i = 0; i < number_of_choices; i++)
		{
            const char *choice;
            gp_widget_get_choice(w, i, &choice);
            choice_value.put_value(choice);
            item_choices.push_back(std::make_pair("", choice_value));
        }
    }



    tree.put("id",              id);
    tree.put("name",            item_name);
    tree.put("label",           item_label);
    tree.put_child("choices",   item_choices);

    switch (type) {
        case GP_WIDGET_TEXT:
        case GP_WIDGET_RADIO:
        case GP_WIDGET_MENU: 
		{
            unsigned char *val = static_cast<unsigned char *>(item_value);
            //tree.put("value", val);
            break;
        }
        case GP_WIDGET_TOGGLE:{
            int val = *((int*)&item_value);
            tree.put<int>("value", val);
            break;
        }

        case GP_WIDGET_RANGE: {
            float val = *((float*)&item_value);
            tree.put<float>("value", val);
            break;
        }

        default:
            break;
    }
}
*/


