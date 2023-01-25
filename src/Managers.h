#pragma once

#include "CommandManager.h"
#include "kernel.h"
#include <functional>

#include "Logging.h"
#include <boost/asio.hpp>

#include <iomanip>
#include <OpenImageDenoise/oidn.hpp>
#include <boost/program_options.hpp>
#include <boost/json.hpp>

#define MESSAGE_HEADER_SIZE 1024

//Forward declaration
class CommandManager;

class Manager {

public:
    CommandManager* cm;
    explicit Manager(CommandManager* _cm);
};


class SceneManager : Manager {

public:

    Scene scene;

    using Manager::Manager;
        
};

class RenderingManager : Manager {

public:

    struct RenderInfo {
        unsigned int samples = 0;
    };

    dev_Scene* dev_scene;

    //TODO: RenderData refactor
    RenderData rd;

    //TODO: See if having two queues is actually necessary or not
    sycl::queue k_q;
    sycl::queue d_q;

    std::thread t_rend;
    std::thread t_key;

    explicit RenderingManager(CommandManager* _cm);

    void start_rendering(Scene* scene);
    float* get_pass(std::string pass);
    RenderInfo get_render_info();                

};



/*
template<typename T>
class Message2 {
public:
    std::vector<T> data;
    Message2(T _data) : data(_data) {}
       
};

template<typename T>
class DataMessage : public Message2<T> {
public:
    DataMessage(T _data) : Message2(_data) {}
    inline static DataMessage<float> FLOAT(float* _data, unsigned int _size) {
        data{_data, _data + _size};
    }
    inline static DataMessage<boost::json::object> JSON(boost::json::object& obj) { return Message2(obj); }
};

class StatusMessage : public Message2<std::string> {
public:
    StatusMessage(std::string status) : Message2(status) {}
    inline static StatusMessage OK() { return StatusMessage("OK"); }
    inline static StatusMessage ERR() { return StatusMessage("ERROR"); }
};

class CommandMessage : public Message2<std::string> {

};*/

struct Message {

    enum class Type { NONE, COMMAND, STATUS, DATA};
    enum class DataFormat { NONE, FLOAT3, FLOAT4, STRING, JSON};

    Type type;
    DataFormat data_format;
    void* data;
    unsigned int data_size;

    Message();

    inline static Message OK() {
        Message ok_message;
        ok_message.data = (void*)("ok");
        ok_message.data_size = 3;
        ok_message.data_format = DataFormat::STRING;
        ok_message.type = Type::STATUS;
        return ok_message;
    }

    static boost::json::object msg2json_header(Message msg);
    static Message json2header(boost::json::object json);
    static std::string type2str(Type type);
    static Type str2type(std::string str);
    static std::string data_format2str(DataFormat data_format);
    static DataFormat str2data_format(std::string str);


    boost::json::object get_json_data();
    float* get_float_data();
    std::string get_string_data();
    std::string to_string();

};

/*
//TODO: Refactor Message templated.
struct Message {

    enum Type { TYPE_NONE, TYPE_COMMAND, TYPE_STATUS, TYPE_BUFFER, TYPE_RENDER_INFO };
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

    inline static Message OK() {
        Message ok_message;
        ok_message.msg = "ok";
        ok_message.type = Message::TYPE_STATUS;
        return ok_message;
    }

    boost::json::object get_json_data();
    float* get_float_data();
};

*/


class InputManager : Manager {

public:

    std::unique_ptr<boost::asio::ip::tcp::socket> sock;
    boost::system::error_code error;
    boost::asio::io_context io_context;

    static const std::vector<std::pair<std::string, std::string>> load_commands;
    static const std::vector<std::pair<std::string, std::string>> execution_commands;

    using Manager::Manager;

    void run();
    void execute_command_msg(Message msg);
    void run_tcp();
    Message read_message();
    void write_message(Message msg);
};


class DenoiseManager : Manager {

    OIDNDevice device = oidnNewDevice(OIDN_DEVICE_TYPE_DEFAULT);


public:
    using Manager::Manager;
    void denoise(int width, int height, float* raw, float* result);
};