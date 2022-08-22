#pragma once

#include "CommandManager.h"
#include "Window.h"
#include "kernel.h"
#include <functional>

#include "Logging.h"
#include <boost/asio.hpp>

#include <iomanip>
#include <OpenImageDenoise/oidn.hpp>
#include <boost/program_options.hpp>
#include <boost/assign.hpp>
#include <boost/json.hpp>

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



struct Message {

    enum Type { TYPE_NONE, TYPE_COMMAND, TYPE_STATUS, TYPE_BUFFER };
    enum DataType { DATA_TYPE_NONE, DATA_TYPE_FLOAT, DATA_TYPE_JSON, DATA_TYPE_STRING };

    static std::map<std::string, Type> type_map;
    static std::map<std::string, DataType> data_type_map;

    Type type;
    std::string msg;

    DataType data_type;
    unsigned int data_size;
    void* data;

    Message();

    static boost::json::object parse_message(Message msg);
    static Message parse_json(boost::json::object json);
    static std::string type2str(Type type);
    static Type str2type(std::string str);
    static std::string data_type2str(DataType data_type);
    static DataType str2data_type(std::string str);

    boost::json::object get_json_data();
};

class InputManager : Manager {

public:

    std::unique_ptr<boost::asio::ip::tcp::socket> sock;
    boost::system::error_code error;
    boost::asio::io_context io_context;

    using Manager::Manager;

    void run();
    std::string execute_command(Message msg);
    void execute_message(Message msg);
    void run_tcp();
    Message read_message();
    void write_message(Message msg);
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