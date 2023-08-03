#include "CommandManager.h"
#include "Logging.h"
#include "TCPInterface.h"




std::vector<const char*> str_to_argv(std::string str) {
    std::string s;
    std::vector<const char*> argv;
    std::istringstream iss(str);
    argv.push_back("ElevenRender");
    while (iss >> std::quoted(s)) {
        char* c = new char[s.size() + 1];
        strcpy(c, s.c_str());
        argv.push_back(c);
    }
    return argv;
}


InputCommand* parse_input_command(Message msg, TCPInterface& tcp_interface) {
    
    std::string command_str = msg.get_string_data();
    std::vector<const char*> argv = str_to_argv(command_str); 

    namespace po = boost::program_options;
    po::variables_map vm;

    po::options_description desc("Allowed options");

    const std::vector<std::pair<std::string, std::string>> load_commands = {
       {"load_config", "load configuration from tcp (use --path for importing .json config files)"},
       {"load_object", "load object from tcp (use --path for importing wavefront .obj files)" },
       {"load_camera", "load camera from tcp (use --path for importing camera .json files)" },
       {"load_hdri", "load HDRI from tcp (use --path for importing .exr/.hdr files)"},
       {"load_brdf_material", "load brdf material from tcp (use --path for importing .brdf files)"},
       {"load_osl_material", "load osl material from tcp (use --path for importing .oslm files)"},
        };

    const std::vector<std::pair<std::string, std::string>> execution_commands = {
        {"start", "start rendering"},
        {"pause", "pause rendering"},
        {"stop", "stop rendering"}
    };

    for (const auto& [k, v] : load_commands)
        desc.add_options()(k.c_str(), v.c_str());

    for (const auto& [k, v] : execution_commands)
        desc.add_options()(k.c_str(), v.c_str());

    desc.add_options()
        ("help", "produce help message")

        //TODO: make this with subcommand
        ("path", po::value<std::vector<std::string>>()->multitoken(), "filesystem path where to load data")
        ("recompute_normals", "when loading an object, choose if the normals will be recomputed")
        ("mirror_x", "when loading texture, flip horizontal pixels")
        ("mirror_y", "when loading texture, flip vertical pixels")
        ("output", po::value<std::string>(), "filesystem path where to output data");

    po::store(po::parse_command_line(argv.size(), argv.data(), desc), vm);
    po::notify(vm);

    InputCommand* ic = nullptr;

    if (vm.count("path")) {
        if (vm.count("load_camera"))
            ic = new CameraDiskLoadInputCommand(vm["path"].as<std::string>());
        if (vm.count("load_texture"))
            ic = new TextureDiskLoadInputCommand(vm["path"].as<std::string>());
    }
    else if (vm.count("sm")) {
        LOG(error) << "Shared memory not implemented yet!";
    }
    else {
        Message data_msg = tcp_interface.read_message();
        if (vm.count("load_camera"))
            ic = new CameraTCPLoadInputCommand(data_msg);
        if (vm.count("load_texture"))
            ic = new TextureTCPLoadInputCommand(data_msg);
    }

    if (ic == nullptr) {
        LOG(error) << "Something went wrong when parsing InputCommands!";
    }

    return ic;
}

int main(int argc, char* argv[]) {

    logging::init();

    TCPInterface tcp_interface;
    CommandManager cm;
    cm.im = &tcp_interface;

    using boost::asio::ip::tcp;

    tcp::acceptor a = tcp::acceptor(tcp_interface.io_context, tcp::endpoint(tcp::v4(), 5557));

    while (1) {
        LOG(info) << "Awaiting for a connection";
        tcp_interface.sock = std::make_unique<boost::asio::ip::tcp::socket>(a.accept());
        LOG(info) << "Connected";

        while (!tcp_interface.error) {
            LOG(debug) << "Awaiting for InputCommand";
            Message msg = tcp_interface.read_message();
            LOG(debug) << "Command message readed ";
            if (msg.type == Message::Type::COMMAND) {
                InputCommand* ic = parse_input_command(msg, tcp_interface);
                cm.execute_input_command(ic);
            }
            else {
                LOG(error) << "Message recieved, expected a command but it's not";
            }
        }
        LOG(info) << "Disconnected";
    }

    LOG(info) << "Quitting";

    return 0;
}
