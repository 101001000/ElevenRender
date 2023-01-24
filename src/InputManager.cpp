#include "CommandManager.h"
#include "Managers.h"
#include <numeric>
#include <string>
#include <algorithm>

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

std::string vec2str(std::vector<std::string> v) {
    auto space = [](std::string a, std::string b)
    {
        return a + ' ' + b;
    };
    std::string result = std::accumulate(v.begin(), v.end(), std::string(""), space);
    result.erase(0, 1);
    return result;
}

template<class T>
class LoadCommand {
public:
    boost::program_options::variables_map& vm;
    CommandManager& cm;
    InputManager& im;
    std::string command;
    T object;

    LoadCommand(boost::program_options::variables_map& _vm, CommandManager& _cm, InputManager& _im, std::string _command) : vm(_vm), cm(_cm), im(_im), command(_command) {};

    virtual T path_load() {};
    virtual T sm_load() {};
    virtual T tcp_load(InputManager& im) {};
    virtual std::function<void()> bind() {};

    std::function<void()> execute() {
        if (vm.count(command)) {
            if (vm.count("path")) {
                object = path_load();
            }                                                                          
            else if (vm.count("sm")) {
                object = sm_load();
            }
            else {
                object = tcp_load(im);
            }
        }
        return bind();
    }
};

class LoadCameraCommand : public LoadCommand<Camera> {
    
    using LoadCommand::LoadCommand;

    Camera parse_camerajson(boost::json::object camera_json) {
        Camera camera;
        boost::json::object position_json = camera_json["position"].as_object();
        boost::json::object rotation_json = camera_json["rotation"].as_object();
        camera.aperture = camera_json["aperture"].as_double();
        camera.bokeh = camera_json["bokeh"].as_bool();
        camera.focusDistance = camera_json["focus_distance"].as_double();
        camera.focalLength = camera_json["focal_length"].as_double();
        camera.sensorWidth = camera_json["sensor_width"].as_double();
        camera.sensorHeight = camera_json["sensor_height"].as_double();
        camera.position = Vector3(position_json["x"].as_double(), position_json["y"].as_double(), position_json["z"].as_double());
        camera.rotation = Vector3(rotation_json["x"].as_double(), rotation_json["y"].as_double(), rotation_json["z"].as_double());
        return camera;
    }

    Camera path_load() {
        std::ifstream st(vm["path"].as<std::string>());
        std::string str((std::istreambuf_iterator<char>(st)), std::istreambuf_iterator<char>());
        return parse_camerajson(boost::json::parse(str).as_object());
    }

    Camera tcp_load(InputManager& im) {
        Message data_msg = im.read_message();
        return parse_camerajson(data_msg.get_json_data());
    }

    std::function<void()> bind() {
        return std::bind(&CommandManager::load_camera, std::ref(cm), object);
    }
};


class LoadConfigCommand : public LoadCommand<RenderParameters> {

    using LoadCommand::LoadCommand;

    RenderParameters tcp_load(InputManager& im) {
        Message data_msg = im.read_message();
        boost::json::object json_data = data_msg.get_json_data();
        try {
            //TODO add max_bounces
            return RenderParameters(json_data["x_res"].as_int64(), json_data["y_res"].as_int64(), json_data["sample_target"].as_int64(), json_data["denoise"].as_bool());
        }
        catch (std::exception const& e) {
            BOOST_LOG_TRIVIAL(error) << "Invalid config format: " << e.what();
        }
    }

    std::function<void()> bind() {
        return std::bind(&CommandManager::load_config, std::ref(cm), object);
    }
};

void InputManager::execute_command_msg(Message msg) {

    std::string command = msg.get_string_data();

    BOOST_LOG_TRIVIAL(trace) << "in InputManager::execute_command" << command;
    std::ostringstream response;

    namespace po = boost::program_options;

    po::options_description desc("Allowed options");

    desc.add_options()
        ("help", "produce help message")

        ("load_config", "load configuration from tcp (use --path for importing .json config files)")
        ("load_object", "load object from tcp (use --path for importing wavefront .obj files)")
        ("load_camera", "load camera from tcp (use --path for importing camera .json files)")
        ("load_hdri", "load HDRI from tcp (use --path for importing .exr/.hdr files)")
        ("load_brdf_material", "load brdf material from tcp (use --path for importing .brdf files)")
        ("load_osl_material", "load osl material from tcp (use --path for importing .oslm files)")
        //("load_matx_material", "load materialx from tcp (use --path for importing .matx files)")
        ("load_texture", "load texture from tcp")

        ("get_info", "get render info and send it through tcp (or save it with --output)")
        ("get_pass", po::value<std::string>(), "[beauty|normal] get some pass and send it through tcp (or save it with --output)")

        ("start", "start rendering")
        ("pause", "pause rendering")
        ("stop", "stop rendering")

        //TODO: make this with subcommand
        ("path", po::value<std::vector<std::string>>()->multitoken(), "filesystem path where to load data")
        ("recompute_normals", "when loading an object, choose if the normals will be recomputed")
        ("mirror_x", "when loading texture, flip horizontal pixels")
        ("mirror_y", "when loading texture, flip vertical pixels")
        ("output", po::value<std::string>(), "filesystem path where to output data")
        ;

    try {

        po::variables_map vm;
        std::function <void()> f;
        std::vector<const char*> argv = str_to_argv(command);

        po::store(po::parse_command_line(argv.size(), argv.data(), desc), vm);
        po::notify(vm);
             





        if (vm.count("load_camera"))
            f = LoadCameraCommand(vm, *cm, *this, "load_camera").execute();

        else if (vm.count("load_config"))
            f = LoadConfigCommand(vm, *cm, *this, "load_config").execute();

        else if (vm.count("help")) {
            response << desc;
        }
        else if (vm.count("load_object")) {
            BOOST_LOG_TRIVIAL(trace) << "InputManager::execute_command -> enqueue load_object ";
            std::vector<MeshObject> objects(0);

            if (vm.count("path")) {
                std::vector<UnloadedMaterial> umtls(0);
                ObjLoader objLoader;
                std::string path_string = vec2str(vm["path"].as<std::vector<std::string>>());
                path_string.erase(remove(path_string.begin(), path_string.end(), '\"'), path_string.end()); // Remove double quotes
                objLoader.loadObjsRapid(path_string, objects, umtls, vm.count("recompute_normals"));
            }
            else if (vm.count("sm")) {
                BOOST_LOG_TRIVIAL(error) << "shared memory feature not implemented yet";
                //TODO implement sm object loading
            }
            else {
                //TODO implement TCP object loading�
                BOOST_LOG_TRIVIAL(error) << "TCP feature not implemented yet";
            }

            f = std::bind(&CommandManager::load_objects, std::ref(cm), objects);
        }

 
        else if (vm.count("load_hdri")) {
            BOOST_LOG_TRIVIAL(trace) << "InputManager::execute_command -> enqueue load_hdri";

            Message metadata_msg = read_message();

            boost::json::object json_metadata = metadata_msg.get_json_data();

            int width = json_metadata["width"].as_int64();
            int height = json_metadata["height"].as_int64();
            int channels = json_metadata["channels"].as_int64();

            write_message(Message::OK());

            Texture tex;

            if (vm.count("path")) {
                //TODO implement HDRI loading
            }
            else if (vm.count("sm")) {
                BOOST_LOG_TRIVIAL(error) << "shared memory feature not implemented yet";
                //TODO implement sm object loading
            }
            else {
                Message data_msg = read_message();
                float* data = data_msg.get_float_data();
                tex = Texture(width, height, channels, data);
            }

            if(vm.count("mirror_x"))
                tex.mirror_x();

            if (vm.count("mirror_y"))
                tex.mirror_y();

            f = std::bind(&CommandManager::load_hdri, std::ref(cm), HDRI(tex));
        }

        else if (vm.count("load_config")) {
            BOOST_LOG_TRIVIAL(trace) << "InputManager::execute_command -> enqueue load_config";
            RenderParameters rp;
            if (vm.count("path")) {
                BOOST_LOG_TRIVIAL(error) << "load_config filesystem feature not implemented yet";
            }
            else if (vm.count("sm")) {
                BOOST_LOG_TRIVIAL(error) << "shared memory feature not implemented yet";
                //TODO implement sm object loading
            }
            else {
                Message data_msg = read_message();
                boost::json::object json_data = data_msg.get_json_data();
                try {
                    //TODO add max_bounces
                    rp = RenderParameters(json_data["x_res"].as_int64(), json_data["y_res"].as_int64(), json_data["sample_target"].as_int64(), json_data["denoise"].as_bool());
                }
                catch (std::exception const& e) {
                    BOOST_LOG_TRIVIAL(error) << "Invalid config format: " << e.what();
                }
            }
            f = std::bind(&CommandManager::load_config, std::ref(cm), rp);
        }

        //TODO: Separate json from material
        else if (vm.count("load_brdf_material")) {
            BOOST_LOG_TRIVIAL(trace) << "InputManager::execute_command -> enqueue load_brdf_material";

            Material material;
            boost::json::object material_json;

            if (vm.count("path")) {
                //TODO implement config loading
            }
            else if (vm.count("sm")) {
                BOOST_LOG_TRIVIAL(error) << "shared memory feature not implemented yet";
                //TODO implement sm object loading
            }
            else {
                Message data_msg = read_message();
                material_json = data_msg.get_json_data();
                
            }
            f = std::bind(&CommandManager::load_material_from_json, std::ref(cm), material_json);
        }


        else if (vm.count("load_osl_material")) {
            //TODO
            if (vm.count("path")) {

            }
            else if (vm.count("sm")) {
                BOOST_LOG_TRIVIAL(error) << "shared memory feature not implemented yet";
                //TODO implement sm object loading
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
            BOOST_LOG_TRIVIAL(trace) << "InputManager::execute_command -> enqueue stop";
            f = std::bind(&CommandManager::stop_render, std::ref(cm));
        }

        else if (vm.count("pause")) {
            //TODO
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

            // 1: check args, send ok
            // 2: wait for metadata
            // 3: check metadata, send ok
            // 4: wait for texture data
            // 5: generate texture

            Message metadata_msg = read_message();

            boost::json::object json_metadata = metadata_msg.get_json_data();

            int width = json_metadata["width"].as_int64();
            int height = json_metadata["height"].as_int64();
            int channels = json_metadata["channels"].as_int64();

            std::string name = json_metadata["name"].as_string().c_str();
            std::string color_space = json_metadata["color_space"].as_string().c_str();

            write_message(Message::OK());

            Texture tex;

            if (vm.count("path")) {

                //TODO: Manage color space loading
                stbi_set_flip_vertically_on_load(true);

                int l_width, l_height, l_channels;

                float* tmp_data = stbi_loadf(vm["path"].as<std::string>().c_str(), &l_width, &l_height, &l_channels, 0);

                //TODO: Add error loading handling
                BOOST_LOG_TRIVIAL(debug) << "Loaded texture from" << vm["path"].as<std::string>();

                if (width != l_width || height != l_height || channels != l_channels) {
                    BOOST_LOG_TRIVIAL(warning) << "Metadata - file metadata missmatch on " << name;
                }

                tex = Texture(l_width, l_height, l_channels, tmp_data);

                stbi_image_free(tmp_data);
            }
            else if (vm.count("sm")) {
                BOOST_LOG_TRIVIAL(error) << "shared memory feature not implemented yet";
                //TODO implement sm object loading
            }
            else {
                Message data_msg = read_message();
                float* data = data_msg.get_float_data();
                //TODO: Fix this constructor arguments. Well, every constructor in reality.
                //TODO: Apply colorspace in loading
                tex = Texture(width, height, channels, data);
                tex.name = name;
            }

            if (vm.count("mirror_x"))
                tex.mirror_x();

            if (vm.count("mirror_y"))
                tex.mirror_y();

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

    std::string header_str = serialize(Message::msg2json_header(msg));

    if (header_str.size() > MESSAGE_HEADER_SIZE)
        BOOST_LOG_TRIVIAL(error) << "TCP header size exceded.";

    // Fill with empty characters to fit header size.
    header_str += std::string(MESSAGE_HEADER_SIZE - header_str.size(), '\0');

    BOOST_LOG_TRIVIAL(trace) << "Writting message with header: " << header_str;
    boost::asio::write(*(sock.get()), boost::asio::buffer(header_str, MESSAGE_HEADER_SIZE));

    if (msg.data_size != 0 &&
        msg.data_format != Message::DataFormat::NONE &&
        msg.data != nullptr) {
        BOOST_LOG_TRIVIAL(trace) << "Writting additional data: " << msg.data_size << " bytes";
        boost::asio::write(*(sock.get()), boost::asio::buffer((char*)(msg.data), msg.data_size));
    }
}


Message InputManager::read_message() {
    Message msg;
    char input_data[MESSAGE_HEADER_SIZE];
    size_t header_size = sock.get()->read_some(boost::asio::buffer(input_data), error);

    if (header_size != MESSAGE_HEADER_SIZE)
        BOOST_LOG_TRIVIAL(error) << "Header size mismatch: " << header_size << " bytes";

    std::string header_str(input_data);
    BOOST_LOG_TRIVIAL(trace) << "Reading message with header: " << header_str;

    try {
        boost::json::value input_json = boost::json::parse(header_str);
        msg = Message::json2header(input_json.as_object());

        if (msg.data_size != 0) {
            BOOST_LOG_TRIVIAL(trace) << "InputManager::read_message() -> reading additional " << msg.data_size << "bytes";
            // TODO: RAII
            msg.data = malloc(msg.data_size);
            boost::asio::read(*(sock.get()), boost::asio::buffer(msg.data, msg.data_size));
        }
    }
    catch (std::exception const& e) {
        BOOST_LOG_TRIVIAL(error) << "Error parsing message. " << e.what();
    }

    return msg;
}

void InputManager::run_tcp() {
    using boost::asio::ip::tcp;
    tcp::acceptor a = tcp::acceptor(io_context, tcp::endpoint(tcp::v4(), 5557));

    // TODO: break condition.
    // TODO: parametrize IP.
    while (1) {
        BOOST_LOG_TRIVIAL(info) << "Awaiting for a connection";
        sock = std::make_unique<boost::asio::ip::tcp::socket>(a.accept());
        BOOST_LOG_TRIVIAL(info) << "Connected";

        while (!error) {
            BOOST_LOG_TRIVIAL(debug) << "Awaiting for command";
            Message msg = read_message();
            BOOST_LOG_TRIVIAL(debug) << "Command message readed ";
            if (msg.type == Message::Type::COMMAND) {
                execute_command_msg(msg);
            }
            else {
                BOOST_LOG_TRIVIAL(error) << "Message recieved, but not a command.";
            }
        }
        BOOST_LOG_TRIVIAL(info) << "Disconnected";
    }
}



