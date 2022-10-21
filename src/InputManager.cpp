#include "CommandManager.h"
#include "Managers.h"


std::vector<const char*> str_to_argv(std::string str) {
    BOOST_LOG_TRIVIAL(trace) << "in str_to_argv()";
    std::string s;
    std::vector<const char*> argv;
    std::istringstream iss(str);
    argv.push_back("ElevenRender");
    while (iss >> std::quoted(s)) {
        char* c = new char[s.size() + 1];
        strcpy(c, s.c_str());
        argv.push_back(c);
    }
    BOOST_LOG_TRIVIAL(trace) << "out str_to_argv()";
    return argv;
}

void InputManager::execute_command_msg(Message msg) {

    std::string command = msg.msg;
    BOOST_LOG_TRIVIAL(trace) << "in InputManager::execute_command" << command;
    std::ostringstream response;

    namespace po = boost::program_options;

    po::options_description desc("Allowed options");

    desc.add_options()
        ("help", "produce help message")

        ("load_config", "load configuration from tcp (use --path for importing .json config files)")
        ("load_object", "load object from tcp (use --path for importing wavefront .obj files)")
        ("load_hdri", "load HDRI from tcp (use --path for importing .exr/.hdr files)")
        ("load_brdf_material", "load brdf material from tcp (use --path for importing .brdf files)")
        ("load_osl_material", "load osl material from tcp (use --path for importing .oslm files)")
        //("load_matx_material", "load materialx from tcp (use --path for importing .matx files)")
        ("load_texture", po::value<std::vector<std::string>>()->multitoken(), "load texture from tcp")

        ("get_info", "get render info and send it through tcp (or save it with --output)")
        ("get_pass", po::value<std::string>(), "[beauty|normal] get some pass and send it through tcp (or save it with --output)")

        ("start", "start rendering")
        ("pause", "pause rendering")
        ("stop", "stop rendering")

        //TODO: make this with subcommand
        ("path", po::value<std::string>(), "filesystem path where to load data")
        ("output", po::value<std::string>(), "filesystem path where to output data")
        ;

    try {

        po::variables_map vm;
        std::function <void()> f;
        std::vector<const char*> argv = str_to_argv(command);

        po::store(po::parse_command_line(argv.size(), argv.data(), desc), vm);
        po::notify(vm);

        if (vm.count("help")) {
            response << desc;
        }

        else if (vm.count("load_object")) {
            BOOST_LOG_TRIVIAL(trace) << "InputManager::execute_command -> enqueue load_object";
            if (vm.count("path")) {
                //TODO make scene object agnostic
                f = std::bind(&CommandManager::load_scene_from_obj, std::ref(cm), vm["path"].as<std::string>());
            }
            else {
                //TODO implement TCP object loading
            }
        }

        else if (vm.count("load_hdri")) {
            BOOST_LOG_TRIVIAL(trace) << "InputManager::execute_command -> enqueue load_hdri";
            if (vm.count("path")) {
                //TODO implement HDRI loading
            }
            else {
                //TODO implement TCP HDRI loading
            }
        }

        else if (vm.count("load_config")) {
            BOOST_LOG_TRIVIAL(trace) << "InputManager::execute_command -> enqueue load_config";
            if (vm.count("path")) {
                //TODO implement config loading
            }
            else {
                //TODO implement TCP config loading
            }
        }

        else if (vm.count("load_brdf_material")) {
            BOOST_LOG_TRIVIAL(trace) << "InputManager::execute_command -> enqueue load_brdf_material";
            if (vm.count("path")) {
                //TODO implement config loading
            }
            else {
                f = std::bind(&CommandManager::load_material_from_json, std::ref(cm), msg.get_json_data());
            }
        }


        else if (vm.count("load_osl_material")) {
            //TODO
            if (vm.count("path")) {

            }
            else {

            }
        }

        else if (vm.count("get_pass")) {
            BOOST_LOG_TRIVIAL(trace) << "InputManager::execute_command -> enqueue get_pass";
            std::string pass = vm["get_pass"].as<std::string>();
            if (vm.count("output")) {
                //TODO: implement save pass
            }
            else {
                f = std::bind(&CommandManager::get_pass, std::ref(cm), pass);
            }
        }

        else if (vm.count("start")) {
            BOOST_LOG_TRIVIAL(trace) << "InputManager::execute_command -> enqueue start";
            f = std::bind(&CommandManager::start_render, std::ref(cm));
        }

        else if (vm.count("stop")) {
            //TODO
        }

        else if (vm.count("pause")) {
            //TODO
        }

        else if (vm.count("start")) {
            BOOST_LOG_TRIVIAL(trace) << "InputManager::execute_command -> enqueue start";
            f = std::bind(&CommandManager::start_render, std::ref(cm));
        }


        else if (vm.count("get_info")) {
            BOOST_LOG_TRIVIAL(trace) << "InputManager::execute_command -> enqueue get_info";
            if (vm.count("output")) {
                //TODO: implement save info as json
            }
            else {
                f = std::bind(&CommandManager::get_render_info, std::ref(cm));
            }
        }

        else if (vm.count("load_texture")) {
            BOOST_LOG_TRIVIAL(trace) << "InputManager::execute_command -> enqueue load_texture";

            Texture tex;

            if (vm.count("path")) {
                //TODO: load texture by file
            }
            else {
                //TODO: change the metadata to a full json message block
                std::vector<std::string> split_metadata = vm["load_texture"].as<std::vector<std::string>>();
                std::string metadata;

                for (std::string str : split_metadata) {
                    metadata += str + " ";
                }

                boost::json::object json = boost::json::parse(metadata).as_object();

                float* msg_data = msg.get_float_data();
                tex = Texture(json["width"].as_int64(), json["height"].as_int64(), 4, msg_data);
                delete[] msg_data;

                tex.name = json["name"].as_string();               

                if (json["color_space"] == "sRGB") {
                    BOOST_LOG_TRIVIAL(debug) << "Applying gamma correction to " << tex.name;
                    tex.applyGamma(2.2);
                }
            }
            f = std::bind(&CommandManager::load_texture, std::ref(cm), tex);
        } 
        
        else {
            BOOST_LOG_TRIVIAL(error) << "Command not found";
        }

        if (f) {
            cm->command_queue.push(f);
        }

        for (const char* c : argv) {
            if (strcmp(c, "ElevenRender") != 0) {
                delete[] c;
            }
        }
    }
    catch (std::exception const& e) {
        BOOST_LOG_TRIVIAL(error) << e.what();
    }

    BOOST_LOG_TRIVIAL(trace) << "out InputManager::execute_command";
}

void InputManager::write_message(Message msg) {
    BOOST_LOG_TRIVIAL(trace) << "in InputManager::write_message()";

    std::string str = serialize(Message::parse_message(msg));
    size_t a = sock.get()->write_some(boost::asio::buffer(str));

    if (msg.data_size != 0 &&
        msg.data_type != Message::DataType::DATA_TYPE_NONE &&
        msg.data != nullptr) {
        BOOST_LOG_TRIVIAL(trace) << "InputManager::write_message -> writting additional data";
        boost::asio::write(*(sock.get()), boost::asio::buffer(static_cast<float*>(msg.data), msg.data_size));
        delete[] msg.data;
    }
    BOOST_LOG_TRIVIAL(trace) << "out InputManager::write_message()";
}

Message InputManager::read_message() {
    BOOST_LOG_TRIVIAL(trace) << "in InputManager::read_message()";

    Message msg;

    char input_data[TCP_MESSAGE_MAXSIZE];

    sock.get()->read_some(boost::asio::buffer(input_data), error);

    std::string input_str(input_data);

    boost::json::value input_json;

    try {
        input_json = boost::json::parse(input_str);

        msg = Message::parse_json(input_json.as_object());

        if (msg.data_size != 0) {
            BOOST_LOG_TRIVIAL(trace) << "InputManager::read_message() -> reading additional data";
            msg.data = malloc(msg.data_size);
            boost::asio::read(*(sock.get()), boost::asio::buffer(msg.data, msg.data_size));
        }
    }
    catch (std::exception const& e) {
        BOOST_LOG_TRIVIAL(error) << e.what();
    }

    BOOST_LOG_TRIVIAL(trace) << "out InputManager::read_message()";
    return msg;
}

void InputManager::run_tcp() {
    BOOST_LOG_TRIVIAL(trace) << "int InputManager::run_tcp()";

    using boost::asio::ip::tcp;
    tcp::acceptor a = tcp::acceptor(io_context, tcp::endpoint(tcp::v4(), 5557));

    while (1) {
        BOOST_LOG_TRIVIAL(info) << "Awaiting for a connection";
        sock = std::make_unique<boost::asio::ip::tcp::socket>(a.accept());
        BOOST_LOG_TRIVIAL(info) << "Connected";

        while (!error) {
            Message msg = read_message();
            BOOST_LOG_TRIVIAL(debug) << "Message readed " << msg.msg;
            if (msg.type == Message::Type::TYPE_COMMAND)
                execute_command_msg(msg);
        }
        BOOST_LOG_TRIVIAL(info) << "Disconnected";
    }
    BOOST_LOG_TRIVIAL(trace) << "out InputManager::run_tcp()";
}

