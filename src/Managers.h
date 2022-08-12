#pragma once

#include "CommandManager.h"
#include "Window.h"
#include "kernel.h"
#include <functional>

#include <iomanip>
#include <OpenImageDenoise/oidn.hpp>
#include <boost/program_options.hpp>
#include <boost/asio.hpp>
#include <boost/assign.hpp>
#include <boost/log/trivial.hpp>

//Forward declaration
class CommandManager;


class Manager {

public:
    CommandManager* cm;
    Manager(CommandManager* _cm);
};


class SceneManager : Manager {
    
public:
    Scene scene;

    using Manager::Manager;
};

class RenderingManager : Manager {
    
public:

    dev_Scene* dev_scene;

    //TODO: RenderData refactor
    RenderData rd;
    sycl::queue q;

    std::thread t_rend;
    std::thread t_key;

    RenderingManager(CommandManager* _cm);

    void start_rendering(Scene* scene);
    float* get_pass(std::string pass);

};

class InputManager : Manager {

public:
    using Manager::Manager;
    void run();
    std::string execute_command(std::string);
    void run_tcp();
};


struct Message {

    enum Type { COMMAND, STATUS };
    enum DataType { NONE, FLOAT, JSON, STRING };

    static std::map<std::string, Type> type_map;
    static std::map<std::string, DataType> data_type_map;

    Type type;
    std::string msg;

    DataType data_type;
    unsigned int data_size;
    void* data;

    static RSJresource parse_message(Message msg);
    static Message parse_json(RSJresource json);

    static std::string parse_type(Type type);
    static std::string parse_data_type(DataType data_type);
};

/*

class WindowManager : Manager {

    Window window;
    PixelBuffer pb;

public:

    std::mutex start_window_mutex;
    bool start_window = false;

    WindowManager(CommandManager* _cm);

    void run();
    void start();
    void set_preview_data(float* data);

};
*/
class DenoiseManager : Manager {

    OIDNDevice device = oidnNewDevice(OIDN_DEVICE_TYPE_DEFAULT);

    
public:
    using Manager::Manager;
    void denoise(int width, int height, float* raw, float* result);
};