#include "CommandManager.h"
#include "Managers.h"
#include "kernel.h"

std::map<std::string, Message::Type> Message::type_map = boost::assign::map_list_of("command", COMMAND)("status", STATUS);
std::map<std::string, Message::DataType> Message::data_type_map = boost::assign::map_list_of("none", NONE)("float", FLOAT)("json", JSON)("string", STRING);

std::string Message::parse_type(Type type) {

    std::string str;

    switch (type) {
    case Type::COMMAND:
        str = "command";
        break;
    case Type::STATUS:
        str = "status";
        break;
    default:
        str = "none";
        break;
    }

    return str;
}

std::string Message::parse_data_type(DataType data_type) {

    std::string str;

    switch (data_type) {
    case DataType::FLOAT:
        str = "float";
        break;
    case DataType::STRING:
        str = "string";
        break;
    case DataType::JSON:
        str = "json";
        break;
    default:
        str = "none";
        break;
    }

    return str;
}

RSJresource Message::parse_message(Message msg) {

    RSJresource json;

    json["message_type"] = Message::parse_type(msg.type);
    json["message"] = msg.msg;

    if (msg.data_size != 0 &&
        msg.data_type != DataType::NONE &&
        msg.data != nullptr) {

        RSJresource additional_data_json;

        additional_data_json["data_size"] = msg.data_size;
        additional_data_json["data_type"] = msg.data_type;

        json["additional_data"] = additional_data_json;
    }
  
    return json;
}

Message Message::parse_json(RSJresource json) {

    Message msg;

    msg.type = type_map[json["message_type"].as_str()];
    msg.msg = json["message"].as_str();
    
    if (json["additional_data"].exists()) {
        RSJresource additional_data_json = json["additional_data"];
        msg.data_size = additional_data_json["data_size"].as<int>();
        msg.data_type = data_type_map[additional_data_json["data_type"].as_str()];
        msg.data = nullptr;
    }

    return msg;
}

std::string InputManager::execute_command(std::string command) {

    std::ostringstream response;

    namespace po = boost::program_options;

    po::options_description desc("Allowed options");

    desc.add_options()
        ("help", "produce help message")
        ("window", "open a preview window")
        ("preview_pass", po::value<std::string>(), "preview some pass")
        ("start", "start rendering")
        ("stop", "stop rendering")
        ("load_obj", po::value<std::string>(), "load wavefront obj from file path")
        ("save_pass", po::value<std::vector<std::string>>()->multitoken(), "save rendering pass")
        ("load_config", po::value<std::string>(), "load rendering config from file path")
        ;

    try {

        std::string s;
        po::variables_map vm;

        std::vector<const char*> argv;

        std::istringstream iss(command);

        argv.push_back("ElevenRender");

        while (iss >> std::quoted(s)) {
            char* c = new char[s.size()];
            strcpy(c, s.c_str());
            argv.push_back(c);
        }

        po::store(po::parse_command_line(argv.size(), argv.data(), desc), vm);
        po::notify(vm);

        if (vm.count("help")) {
            response << desc;
        }

        if (vm.count("load_obj")) {
            std::cout << "adding load_obj to the queue";
            std::function <void()> f = std::bind(&CommandManager::load_scene_from_obj, std::ref(cm), vm["load_obj"].as<std::string>());
            cm->command_queue.push(f);
            response << "ok";
        }

        if (vm.count("save_pass")) {
            std::cout << "adding save_pass to the queue";
            std::string pass = vm["save_pass"].as<std::vector<std::string>>()[0];
            std::string path = vm["save_pass"].as<std::vector<std::string>>()[1];
            std::function <void()> f = std::bind(&CommandManager::save_pass, std::ref(cm), pass, path);
            cm->command_queue.push(f);
            response << "ok";
        }

        if (vm.count("start")) {
            std::cout << "adding start to the queue";
            std::function <void()> f = std::bind(&CommandManager::start_render, std::ref(cm));
            cm->command_queue.push(f);
            response << "ok";
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

void write_message(boost::asio::ip::tcp::socket& sock, Message msg, boost::system::error_code& error) {

    std::string str = Message::parse_message(msg).as_str();
    sock.write_some(boost::asio::buffer(str));

    if (msg.data_size != 0 &&
        msg.data_type != Message::DataType::NONE &&
        msg.data != nullptr) {

        boost::asio::write(sock, boost::asio::buffer(msg.data, msg.data_size));
    }
}

Message read_message(boost::asio::ip::tcp::socket& sock, boost::system::error_code& error) {

    Message msg;

    char input_data[TCP_MESSAGE_MAXSIZE];

    sock.read_some(boost::asio::buffer(input_data), error);

    std::string input_str(input_data);

    RSJresource input_json(input_str);

    msg = Message::parse_json(input_json);

    if (msg.data_size != 0) {
        msg.data = malloc(msg.data_size);
        boost::asio::read(sock, boost::asio::buffer(msg.data, msg.data_size));
    }

    return msg;
}

void InputManager::run_tcp() {

    using boost::asio::ip::tcp;
 
    boost::asio::io_context io_context;
    tcp::acceptor a(io_context, tcp::endpoint(tcp::v4(), 5557));

    while (1) {
        std::cout << "Awaiting for a connection" << std::endl;
        tcp::socket sock = a.accept();
        std::cout << "Connected!" << std::endl;

        boost::system::error_code error;
        
        while (!error) {

            std::cout << "trying to read message " << std::endl;
            Message msg = read_message(sock, error);
            std::cout << "Message read " << msg.msg << std::endl;

            if (msg.type == Message::COMMAND) {
                std::cout << "Executing command... " << msg.msg << std::endl;
                std::string result = execute_command(msg.msg);
            }
        }

        std::cout << "Disconnected!" << std::endl;
    }
}



class CUDASelector : public sycl::device_selector {
public:
    int operator()(const sycl::device& device) const override {
        if (device.get_platform().get_backend() == sycl::backend::ext_oneapi_cuda) {
            std::cout << " CUDA device found " << std::endl;
            return 1;
        }
        else {
            return -1;
        }
    }
};

void RenderingManager::start_rendering(Scene* scene) {

    rd.pars = RenderParameters(scene->camera.xRes, scene->camera.yRes, 50);

    for (int i = 0; i < PASSES_COUNT; i++) {

        if (rd.pars.passes_enabled[i]) {
            rd.passes[i] = new float[rd.pars.width * rd.pars.height * 4];
            memset(rd.passes[i], 0,
                rd.pars.width * rd.pars.height * 4 * sizeof(float));
        }
    }

    t_rend = std::thread(renderSetup, std::ref(q), std::ref(scene), std::ref(dev_scene));

    //t_key = std::thread(key_press);

    rd.startTime = std::chrono::high_resolution_clock::now();
}

RenderingManager::RenderingManager(CommandManager* _cm) : Manager(_cm){
    q = sycl::queue(CUDASelector());
    dev_scene = sycl::malloc_device<dev_Scene>(1, q);
}


float* RenderingManager::get_pass(std::string pass) {
    
    int n = parsePass(pass) * rd.pars.width * rd.pars.height * 4;

    printf("\nRetrieving pass %d ...\n", parsePass(pass));

    float* dev_passes;
    float* pass_result = new float[rd.pars.width * rd.pars.height * 4];

    q.memcpy(&dev_passes, &(dev_scene->dev_passes), sizeof(float*)).wait();
    q.memcpy(pass_result, dev_passes + n, rd.pars.width * rd.pars.height * 4 * sizeof(float)).wait();

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

