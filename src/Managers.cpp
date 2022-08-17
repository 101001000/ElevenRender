#include "CommandManager.h"
#include "Managers.h"
#include "kernel.h"

std::map<std::string, Message::Type> Message::type_map = boost::assign::map_list_of("none", TYPE_NONE)("command", TYPE_COMMAND)("status", TYPE_STATUS);
std::map<std::string, Message::DataType> Message::data_type_map = boost::assign::map_list_of("none", DATA_TYPE_NONE)("float", DATA_TYPE_FLOAT)("json", DATA_TYPE_JSON)("string", DATA_TYPE_STRING);

Message::Message() {
    type = Type::TYPE_NONE;
    msg = "";
    data_type = DataType::DATA_TYPE_NONE;
    data_size = 0;
    data = nullptr;
}


RSJresource Message::parse_message(Message msg) {
    BOOST_LOG_TRIVIAL(trace) << "Message::parse_message()";
    RSJresource json;

    for (auto it = type_map.begin(); it != type_map.end(); ++it)
        if (it->second == msg.type)
            json["type"] = it->first;

    json["message"] = msg.msg;

    if (msg.data_size != 0 &&
        msg.data_type != DataType::DATA_TYPE_NONE &&
        msg.data != nullptr) {

        RSJresource additional_data_json;

        additional_data_json["data_size"] = msg.data_size;
        additional_data_json["data_type"] = msg.data_type;

        json["additional_data"] = additional_data_json;
    }
  
    return json;
}

Message Message::parse_json(RSJresource json) {
    BOOST_LOG_TRIVIAL(trace) << "Message::parse_json()";
    Message msg;

    msg.type = type_map[json["type"].as<std::string>()];
    msg.msg = json["msg"].as<std::string>();

    if (json["additional_data"].exists()) {
        RSJresource additional_data_json = json["additional_data"];
        msg.data_size = additional_data_json["data_size"].as<int>();
        msg.data_type = data_type_map[additional_data_json["data_type"].as<std::string>()];
        msg.data = nullptr;
    }

    return msg;
}

std::string InputManager::execute_command(std::string command) {
    BOOST_LOG_TRIVIAL(trace) << "InputManager::execute_command " << command;
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
            BOOST_LOG_TRIVIAL(debug) << "Adding load_obj to the queue";
            std::function <void()> f = std::bind(&CommandManager::load_scene_from_obj, std::ref(cm), vm["load_obj"].as<std::string>());
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
    BOOST_LOG_TRIVIAL(trace) << "write_message()";

    std::string str = Message::parse_message(msg).as<std::string>();
    sock.write_some(boost::asio::buffer(str));

    if (msg.data_size != 0 &&
        msg.data_type != Message::DataType::DATA_TYPE_NONE &&
        msg.data != nullptr) {

        boost::asio::write(sock, boost::asio::buffer(msg.data, msg.data_size));
    }
}

Message read_message(boost::asio::ip::tcp::socket& sock, boost::system::error_code& error) {
    BOOST_LOG_TRIVIAL(trace) << "read_message()";

    char input_data[TCP_MESSAGE_MAXSIZE];

    sock.read_some(boost::asio::buffer(input_data), error);

    std::string input_str(input_data);

    RSJresource input_json(input_str);

    Message msg = Message::parse_json(input_json);

    
    if (msg.data_size != 0) {
        BOOST_LOG_TRIVIAL(trace) << "reading additional data";
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
        BOOST_LOG_TRIVIAL(info) << "Awaiting for a connection";
        tcp::socket sock = a.accept();
        BOOST_LOG_TRIVIAL(info) << "Connected";

        boost::system::error_code error;
        
        while (!error) {

            BOOST_LOG_TRIVIAL(debug) << "Trying to read message";
            Message msg = read_message(sock, error);
            BOOST_LOG_TRIVIAL(debug) << "Message readed ";
                        

            if (msg.type == Message::Type::TYPE_COMMAND) {
                BOOST_LOG_TRIVIAL(debug) << "Executing command " << msg.msg;
                std::string result = execute_command(msg.msg);
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

    BOOST_LOG_TRIVIAL(debug) << "Retrieving pass: " << pass;

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

