#include "CommandManager.h"
#include "Managers.h"
#include "kernel.h"

std::map<std::string, Message::Type> Message::type_map = boost::assign::map_list_of("none", TYPE_NONE)
                                                                                   ("command", TYPE_COMMAND)
                                                                                   ("status", TYPE_STATUS)
                                                                                   ("buffer", TYPE_BUFFER);
std::map<std::string, Message::DataType> Message::data_type_map = boost::assign::map_list_of("none", DATA_TYPE_NONE)("float", DATA_TYPE_FLOAT)("json", DATA_TYPE_JSON)("string", DATA_TYPE_STRING);

Message::Message() {
    type = Type::TYPE_NONE;
    msg = "";
    data_type = DataType::DATA_TYPE_NONE;
    data_size = 0;
    data = nullptr;
}

std::string Message::type2str(Type type) {
    std::string str = "";

    for (auto it = type_map.begin(); it != type_map.end(); ++it)
        if (it->second == type)
            str = it->first;
    return str;
}
Message::Type Message::str2type(std::string str) {
    return type_map[str];
}
std::string Message::data_type2str(DataType data_type) {
    std::string str = "";

    for (auto it = data_type_map.begin(); it != data_type_map.end(); ++it)
        if (it->second == data_type)
            str = it->first;
    return str;
}
Message::DataType Message::str2data_type(std::string str) {
    return data_type_map[str];
}

boost::json::object Message::get_json_data() {
    BOOST_LOG_TRIVIAL(trace) << "Message::get_json_data()";
    boost::json::value json;

    try {
        json = boost::json::parse((char*) data);
    }
    catch (std::exception const& e) {
        BOOST_LOG_TRIVIAL(error) << e.what() << ((char*)data);
    }

    return json.as_object();
}

float* Message::get_float_data() {
    BOOST_LOG_TRIVIAL(trace) << "Message::get_float_data()";
    return (float*) data;
}



boost::json::object Message::parse_message(Message msg) {

    BOOST_LOG_TRIVIAL(trace) << "Message::parse_message()";
    boost::json::object json;

    json["type"] = Message::type2str(msg.type);
    json["msg"] = msg.msg;

    if (msg.data_size != 0 &&
        msg.data_type != DataType::DATA_TYPE_NONE &&
        msg.data != nullptr) {

        boost::json::object additional_data_json;

        additional_data_json["data_size"] = msg.data_size;
        additional_data_json["data_type"] = msg.data_type;

        json["additional_data"] = additional_data_json;
    }

    return json;
}

void InputManager::execute_message(Message msg) {
    if (msg.type == Message::Type::TYPE_COMMAND) {
        BOOST_LOG_TRIVIAL(debug) << "Executing command " << msg.msg;
        std::string result = execute_command(msg);
    }
}

Message Message::parse_json(boost::json::object json) {
    BOOST_LOG_TRIVIAL(trace) << "Message::parse_json()";
    Message msg;

    msg.type = Message::str2type(json["type"].as_string().c_str());
    msg.msg = json["msg"].as_string();

    if (json.if_contains("additional_data")) {
        boost::json::object additional_data_json = json["additional_data"].as_object();
        msg.data_size = additional_data_json["data_size"].as_int64();
        msg.data_type = str2data_type(additional_data_json["data_type"].as_string().c_str());
        msg.data = nullptr;
    }

    return msg;
}

std::string InputManager::execute_command(Message msg) {

    std::string command = msg.msg;

    BOOST_LOG_TRIVIAL(trace) << "InputManager::execute_command " << command;
    std::ostringstream response;

    namespace po = boost::program_options;

    po::options_description desc("Allowed options");

    desc.add_options()
        ("help", "produce help message")
        ("save_pass", po::value<std::vector<std::string>>()->multitoken(), "save some pass")
        ("get_pass", po::value<std::string>(), "get some pass")
        ("start", "start rendering")
        ("stop", "stop rendering")
        ("load_obj", po::value<std::string>(), "load wavefront obj from file path")
        ("load_config", po::value<std::string>(), "load rendering config from file path")
        ("load_material", "load material from tcp")
        ("load_texture", po::value<std::vector<std::string>>()->multitoken(), "load texture from tcp")
        ;

    try {

        std::string s;
        po::variables_map vm;

        std::vector<const char*> argv;

        std::istringstream iss(command);

        argv.push_back("ElevenRender");

        while (iss >> std::quoted(s)) {
            char* c = new char[s.size() + 1];
            strcpy(c, s.c_str());
            argv.push_back(c);
        }

        po::store(po::parse_command_line(argv.size(), argv.data(), desc), vm);
        po::notify(vm);

        if (vm.count("help")) {
            response << desc;
        }

        if (vm.count("load_obj")) {
            BOOST_LOG_TRIVIAL(debug) << "Adding load_obj to the queue";
            std::function <void()> f = std::bind(&CommandManager::load_scene_from_obj, std::ref(cm), vm["load_obj"].as<std::string>());
            cm->command_queue.push(f);
            response << "ok";
        }

        if (vm.count("get_pass")) {
            BOOST_LOG_TRIVIAL(debug) << "Adding get_pass to the queue";
            std::string pass = vm["get_pass"].as<std::string>();
            std::function <void()> f = std::bind(&CommandManager::get_pass, std::ref(cm), pass);
            cm->command_queue.push(f);
            response << "ok";
        }

        if (vm.count("save_pass")) {
            BOOST_LOG_TRIVIAL(debug) << "Adding save_pass to the queue";
            std::string pass = vm["save_pass"].as<std::vector<std::string>>()[0];
            std::string path = vm["save_pass"].as<std::vector<std::string>>()[1];
            std::function <void()> f = std::bind(&CommandManager::save_pass, std::ref(cm), pass, path);
            cm->command_queue.push(f);
            response << "ok";
        }

        if (vm.count("start")) {
            BOOST_LOG_TRIVIAL(debug) << "Adding start to the queue";
            std::function <void()> f = std::bind(&CommandManager::start_render, std::ref(cm));
            cm->command_queue.push(f);
            response << "ok";
        }

        if (vm.count("load_material")) {
            BOOST_LOG_TRIVIAL(debug) << "Adding load material to the queue";

            std::function <void()> f = std::bind(&CommandManager::load_material_from_json, std::ref(cm), msg.get_json_data());
            cm->command_queue.push(f);
            response << "ok";
        }


        if (vm.count("load_texture")) {
            BOOST_LOG_TRIVIAL(debug) << "Adding load texture to the queue";

            std::vector<std::string> split_metadata = vm["load_texture"].as<std::vector<std::string>>();
            std::string metadata;

            for (std::string str : split_metadata) {
                metadata += str + " ";
            }

            BOOST_LOG_TRIVIAL(debug) << metadata;

            boost::json::object json;

            try {

                json = boost::json::parse(metadata).as_object();

                Texture tex;

                tex.name = json["name"].as_string();
                tex.width = json["width"].as_int64();
                tex.height = json["height"].as_int64();
                tex.data = msg.get_float_data();

                if (json["color_space"] == "sRGB")
                    tex.applyGamma(2.2);

                std::function <void()> f = std::bind(&CommandManager::load_texture, std::ref(cm), tex);
                cm->command_queue.push(f);
                response << "ok";
            }
            catch (std::exception const& e) {
                BOOST_LOG_TRIVIAL(error) << e.what() << metadata;
                response << "error";
                return response.str();
            }

        }

        for (const char* c : argv) {

            if (strcmp(c, "ElevenRender") != 0) {
                delete[] c;
            }
        }
        return response.str();
    }
    catch (std::exception const& e) {
        std::cerr << e.what() << "\n";
        return "Error";
    }
    
    return "nothing";
}

void InputManager::write_message(Message msg) {
    BOOST_LOG_TRIVIAL(trace) << "write_message()";

    std::string str = serialize(Message::parse_message(msg));
    size_t a = sock.get()->write_some(boost::asio::buffer(str));

    BOOST_LOG_TRIVIAL(debug) << "Message: " << str;

    if (msg.data_size != 0 &&
        msg.data_type != Message::DataType::DATA_TYPE_NONE &&
        msg.data != nullptr) {
        BOOST_LOG_TRIVIAL(trace) << "writting additional data";
        boost::asio::write(*(sock.get()), boost::asio::buffer((float*) msg.data, msg.data_size));
    }
    BOOST_LOG_TRIVIAL(trace) << "leaving write_message()";
}

Message InputManager::read_message() {
    BOOST_LOG_TRIVIAL(trace) << "read_message()";

    char input_data[TCP_MESSAGE_MAXSIZE];

    sock.get()->read_some(boost::asio::buffer(input_data), error);

    std::string input_str(input_data);

    boost::json::value input_json;

    try {

        input_json = boost::json::parse(input_str);

    }
    catch (std::exception const& e) {
        std::cerr << e.what() << "\n";
    }

    Message msg = Message::parse_json(input_json.as_object());

    if (msg.data_size != 0) {
        BOOST_LOG_TRIVIAL(trace) << "reading additional data";
        msg.data = malloc(msg.data_size);
        boost::asio::read(*(sock.get()), boost::asio::buffer(msg.data, msg.data_size));
    }

    return msg;
}

void InputManager::run_tcp() {

    using boost::asio::ip::tcp;

    tcp::acceptor a = tcp::acceptor(io_context, tcp::endpoint(tcp::v4(), 5557));

 
    while (1) {
        BOOST_LOG_TRIVIAL(info) << "Awaiting for a connection";
        sock = std::make_unique<boost::asio::ip::tcp::socket>(a.accept());
        BOOST_LOG_TRIVIAL(info) << "Connected";
        
        while (!error) {

            Message msg = read_message();
            BOOST_LOG_TRIVIAL(debug) << "Message readed " << msg.msg;

            if (msg.type == Message::Type::TYPE_COMMAND) {
                BOOST_LOG_TRIVIAL(debug) << "Executing command " << msg.msg;
                std::string result = execute_command(msg);
            }
        }

        BOOST_LOG_TRIVIAL(info) << "Disconnected";
    }
}



class CUDASelector : public sycl::device_selector {
public:
    int operator()(const sycl::device& device) const override {
        if (device.get_platform().get_backend() == sycl::backend::ext_oneapi_cuda) {
            return 1;
        }
        else {
            return -1;
        }
    }
};




void RenderingManager::start_rendering(Scene* scene) {

    BOOST_LOG_TRIVIAL(trace) << "RenderingManager::start_rendering()";
    rd.pars = RenderParameters(scene->camera.xRes, scene->camera.yRes, 50);

    for (int i = 0; i < PASSES_COUNT; i++) {

        if (rd.pars.passes_enabled[i]) {
            rd.passes[i] = new float[rd.pars.width * rd.pars.height * 4];
            memset(rd.passes[i], 0,
                rd.pars.width * rd.pars.height * 4 * sizeof(float));
        }
    }

    t_rend = std::thread(renderSetup, std::ref(q), std::ref(scene), std::ref(dev_scene));

    rd.startTime = std::chrono::high_resolution_clock::now();
    BOOST_LOG_TRIVIAL(trace) << "LEAVING RenderingManager::start_rendering()";
}

RenderingManager::RenderingManager(CommandManager* _cm) : Manager(_cm){
    BOOST_LOG_TRIVIAL(trace) << "RenderingManager::RenderingManager()";
    q = sycl::queue(CUDASelector());
    sycl::device device = q.get_device();

    BOOST_LOG_TRIVIAL(info) << "Device selected: " << device.get_info<sycl::info::device::name>();

    dev_scene = sycl::malloc_device<dev_Scene>(1, q);
}


float* RenderingManager::get_pass(std::string pass) {
    
    int n = parsePass(pass) * rd.pars.width * rd.pars.height * 4;

    BOOST_LOG_TRIVIAL(debug) << "Retrieving pass9: " << pass;

    float* dev_passes;
    float* pass_result = new float[rd.pars.width * rd.pars.height * 4];

    BOOST_LOG_TRIVIAL(debug) << "Retrieving pass1: " << pass;

    q.memcpy(&dev_passes, &(dev_scene->dev_passes), sizeof(float*));

    BOOST_LOG_TRIVIAL(debug) << "Retrieving pas2: " << pass;

    q.memcpy(pass_result, dev_passes + n, rd.pars.width * rd.pars.height * 4 * sizeof(float));

    BOOST_LOG_TRIVIAL(debug) << "Pass retrieved!";

    return pass_result;
}


Manager::Manager(CommandManager* _cm) {
    this->cm = _cm;
}

void DenoiseManager::denoise(int width, int height, float* raw, float* result) {
    OIDNFilter filter =
        oidnNewFilter(device, "RT");  // generic ray tracing filter
    oidnSetSharedFilterImage(filter, "color", raw,
        OIDN_FORMAT_FLOAT3, width, height, 0,
        sizeof(float) * 4, 0);  // beauty
    oidnSetFilter1b(filter, "hdr", true);            // beauty image is HDR
    oidnSetSharedFilterImage(filter, "output", result,
        OIDN_FORMAT_FLOAT3, width, height, 0,
        sizeof(float) * 4, 0);  // denoised beauty

    oidnCommitFilter(filter);
    oidnExecuteFilter(filter);

    const char* errorMessage;
    if (oidnGetDeviceError(device, &errorMessage) != OIDN_ERROR_NONE)
        printf("Error: %s\n", errorMessage);

    // Cleanup
    oidnReleaseFilter(filter);
}

