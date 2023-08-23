#include "CommandManager.h"
#include "Logging.h"
#include "TCPInterface.h"
#include <numeric>

#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>

#include "kernel.h"



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

std::string vec2str(std::vector<std::string> v) {
    auto space = [](std::string a, std::string b)
    {
        return a + ' ' + b;
    };
    std::string result = std::accumulate(v.begin(), v.end(), std::string(""), space);
    result.erase(0, 1);
    return result;
}

InputCommand* parse_input_command(Message msg, TCPInterface& tcp_interface) {
    
    std::string command_str = msg.get_string_data();
    std::vector<const char*> argv = str_to_argv(command_str); 

    LOG(debug) << "Parsing: " << command_str;

    namespace po = boost::program_options;
    po::variables_map vm;

    po::options_description desc("Allowed options");

    const std::vector<std::pair<std::string, std::string>> load_commands = {
       {"load_config", "load configuration from tcp (use --path for importing .json config files)"},
       {"load_texture", "load texture from tcp (use --path for importing image files)"},
       {"load_object", "load object from tcp (use --path for importing wavefront .obj files)" },
       {"load_camera", "load camera from tcp (use --path for importing camera .json files)" },
       {"load_hdri", "load HDRI from tcp (use --path for importing .exr/.hdr files)"},
       {"load_brdf_material", "load brdf material from tcp (use --path for importing .brdf files)"},
       {"load_osl_material", "load osl material from tcp (use --path for importing .oslm files)"},
        };

    const std::vector<std::pair<std::string, std::string>> execution_commands = {
        {"start", "start rendering"},
        {"pause", "pause rendering"},
        {"abort", "abort rendering"}
    };

    for (const auto& [k, v] : load_commands)
        desc.add_options()(k.c_str(), v.c_str());

    for (const auto& [k, v] : execution_commands)
        desc.add_options()(k.c_str(), v.c_str());

    LOG(debug) << "3:";

    desc.add_options()
        ("help", "produce help message")

        //TODO: make this with subcommand
        ("path", po::value<std::vector<std::string>>()->multitoken(), "filesystem path where to load data")
        ("recompute_normals", "when loading an object, choose if the normals will be recomputed")
        ("mirror_x", "when loading texture, flip horizontal pixels")
        ("mirror_y", "when loading texture, flip vertical pixels")
        ("output", po::value<std::string>(), "filesystem path where to output data")
        ( "get_info", "get render information" )
        ( "get_sycl_info", "get sycl information")
        ( "get_pass", po::value<std::string>(), "get render pass" );

    po::store(po::parse_command_line(argv.size(), argv.data(), desc), vm);
    po::notify(vm);

    InputCommand* ic = nullptr;


    if (vm.count("path")) {

        std::string path = vec2str(vm["path"].as<std::vector<std::string>>());

        if (vm.count("load_camera"))
            ic = new CameraDiskLoadInputCommand(path);
        if (vm.count("load_texture"))
            ic = new TextureDiskLoadInputCommand(path);
        if (vm.count("load_config"))
            ic = new ConfigDiskLoadInputCommand(path);
        if (vm.count("load_hdri"))
            ic = new HdriDiskLoadInputCommand(path);
        if (vm.count("load_brdf_material"))
            ic = new BrdfDiskLoadInputCommand(path);
        if (vm.count("load_object"))
            ic = new ObjectsDiskLoadInputCommand(path, vm.count("recompute_normals"));
    }
    else if (vm.count("sm")) {

        // QUICK SM PROTOTYPE WHICH NEEDS TO BE REFACTORED TO THEIR OWN INPUT MESSAGE.
        if (vm.count("load_texture")) {
            Message data_msg = tcp_interface.read_message();
            Message extra_data_msg; 
            using namespace boost::interprocess;

            try {
                boost::interprocess::file_mapping m_file("shm", boost::interprocess::read_only);
                boost::interprocess::mapped_region region(m_file, boost::interprocess::read_only);

                extra_data_msg.data = new float[region.get_size() / 4];
                memcpy(extra_data_msg.data, region.get_address(), region.get_size());
                extra_data_msg.type = Message::Type::DATA;
                extra_data_msg.data_size = region.get_size();
                extra_data_msg.data_format = Message::DataFormat::FLOAT4;
            }
            catch (const interprocess_exception& e) {
                LOG(error) << e.what();
            }
            ic = new TextureTCPLoadInputCommand(data_msg, extra_data_msg);
        }
        else {
            LOG(error) << "Shared memory not implemented yet!";
        }
    }
    else {
       
        if (vm.count("load_camera")) {
            Message data_msg = tcp_interface.read_message();
            ic = new CameraTCPLoadInputCommand(data_msg);
        }
        if (vm.count("load_texture")) {
            Message data_msg = tcp_interface.read_message();
            Message extra_data_msg = tcp_interface.read_message();
            ic = new TextureTCPLoadInputCommand(data_msg, extra_data_msg);
        }
        if (vm.count("load_config")) {
            Message data_msg = tcp_interface.read_message();
            ic = new ConfigTCPLoadInputCommand(data_msg);
        }
        if (vm.count("load_hdri")) {
            Message data_msg = tcp_interface.read_message();
            Message extra_data_msg = tcp_interface.read_message();
            ic = new HdriTCPLoadInputCommand(data_msg, extra_data_msg, vm.count("mirror_x"), vm.count("mirror_y"));
        }
        if (vm.count("load_brdf_material")) {
            Message data_msg = tcp_interface.read_message();
            ic = new BrdfTCPLoadInputCommand(data_msg);
        }
        if (vm.count("load_object")) {
            Message data_msg = tcp_interface.read_message();
            Message mat_msg = tcp_interface.read_message();
            LOG(warning) << vm.count("recompute_normals");
            ic = new ObjectsTCPLoadInputCommand(data_msg, mat_msg, vm.count("recompute_normals"));
        }
        if (vm.count("start")) {
            ic = new StartInputCommand();
        }
        if (vm.count("get_info")) {
            ic = new GetInfoInputCommand();
        }
        if (vm.count("get_sycl_info")) {
            ic = new GetSyclInfoInputCommand();
        }
        if (vm.count("get_pass")) {
            ic = new GetPassInputCommand(vm["get_pass"].as<std::string>());
        }
    }

    if (ic == nullptr) {
        LOG(error) << "Input Command not recognized in: " << command_str;
    }
    else {
        LOG(trace) << "Input Command recognized";
    }

    return ic;
}


int main(int argc, char* argv[]) {

    logging::init();

    TCPInterface tcp_interface;
    using boost::asio::ip::tcp;

    LOG(info) << "Starting acceptor";
    tcp::acceptor a = tcp::acceptor(tcp_interface.io_context, tcp::endpoint(tcp::v4(), 5557));
    LOG(info) << "Acceptor started";

    while (1) {
        LOG(info) << "Awaiting for a connection";
        tcp_interface.sock = std::make_unique<boost::asio::ip::tcp::socket>(a.accept());
        LOG(info) << "Connected";

        CommandManager cm;
        cm.im = &tcp_interface;

        tcp_interface.write_message(Message::OK());

        while (!tcp_interface.error) {
            LOG(debug) << "Awaiting for InputCommand";
            Message msg = tcp_interface.read_message();
            LOG(debug) << "Message readed ";
            if (msg.type == Message::Type::COMMAND) {
                InputCommand* ic = parse_input_command(msg, tcp_interface);
                cm.execute_input_command(ic);
            }
            else if (msg.type == Message::Type::STATUS) {
                if (msg.get_string_data() == "close_session") {
                    LOG(info) << "Clossing session";
                    break;
                }
                else {
                    LOG(error) << "Message recieved, expected a command but it's not";
                }
            }
        }
        LOG(info) << "Disconnected";
        // Here I should clean the session.
    }

    LOG(info) << "Quitting";
    
    return 0;
}
