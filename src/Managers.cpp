#include "CommandManager.h"
#include "Managers.h"
#include "kernel.h"

std::map<std::string, Message::Type> Message::type_map = boost::assign::map_list_of("none", TYPE_NONE)
                                                                                   ("command", TYPE_COMMAND)
                                                                                   ("status", TYPE_STATUS)
                                                                                   ("buffer", TYPE_BUFFER)
                                                                                   ("render_info", TYPE_RENDER_INFO);
std::map<std::string, Message::DataType> Message::data_type_map = boost::assign::map_list_of("none", DATA_TYPE_NONE)("float", DATA_TYPE_FLOAT)("json", DATA_TYPE_JSON)("string", DATA_TYPE_STRING);




Message Message::parse_json(boost::json::object json) {
    BOOST_LOG_TRIVIAL(trace) << "in Message::parse_json()";
    Message msg;

    msg.type = Message::str2type(json["type"].as_string().c_str());
    msg.msg = json["msg"].as_string();

    if (json.if_contains("additional_data")) {
        BOOST_LOG_TRIVIAL(trace) << "Message::parse_json() -> parsing additional data";
        boost::json::object additional_data_json = json["additional_data"].as_object();
        msg.data_size = additional_data_json["data_size"].as_int64();
        msg.data_type = str2data_type(additional_data_json["data_type"].as_string().c_str());
        msg.data = nullptr;
    }

    BOOST_LOG_TRIVIAL(trace) << "out Message::parse_json()";
    return msg;
}

Message::Message() {
    type = Type::TYPE_NONE;
    msg = "";
    data_type = DataType::DATA_TYPE_NONE;
    data_size = 0;
    data = nullptr;
}

std::string Message::type2str(Type type) {
    std::string str = "";

    for (auto it = type_map.begin(); it != type_map.end(); ++it) {
        if (it->second == type) {
            str = it->first;
        }
    }
    return str;
}
Message::Type Message::str2type(std::string str) {
    return type_map[str];
}
std::string Message::data_type2str(DataType data_type) {
    std::string str = "";

    for (auto it = data_type_map.begin(); it != data_type_map.end(); ++it) {
        if (it->second == data_type) {
            str = it->first;
        }
    }
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


RenderingManager::RenderInfo RenderingManager::get_render_info() {

    BOOST_LOG_TRIVIAL(debug) << "Getting render info";

    float* dev_samples;
    unsigned int sample_count = 0;

    d_q.memcpy(&dev_samples, &(dev_scene->dev_samples), sizeof(unsigned int*));

    d_q.memcpy(&sample_count, dev_samples, 1 * sizeof(unsigned int));

    RenderInfo render_info;
    render_info.samples = sample_count;

    BOOST_LOG_TRIVIAL(debug) << "Rendering info retrieved";

    return render_info;
}

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

    t_rend = std::thread(renderSetup, std::ref(k_q), std::ref(scene), std::ref(dev_scene));
    t_rend.join();

    rd.startTime = std::chrono::high_resolution_clock::now();
    BOOST_LOG_TRIVIAL(trace) << "LEAVING RenderingManager::start_rendering()";
}

RenderingManager::RenderingManager(CommandManager* _cm) : Manager(_cm){
    BOOST_LOG_TRIVIAL(trace) << "RenderingManager::RenderingManager()";
    k_q = sycl::queue(CUDASelector());
    d_q = sycl::queue(CUDASelector());

    sycl::device device = k_q.get_device();

    BOOST_LOG_TRIVIAL(info) << "Device selected: " << device.get_info<sycl::info::device::name>();

    dev_scene = sycl::malloc_device<dev_Scene>(1, k_q);
}


float* RenderingManager::get_pass(std::string pass) {
    
    int n = parsePass(pass) * rd.pars.width * rd.pars.height * 4;

    BOOST_LOG_TRIVIAL(debug) << "Retrieving pass: " << pass;

    float* dev_passes;
    float* pass_result = new float[rd.pars.width * rd.pars.height * 4];

    d_q.memcpy(&dev_passes, &(dev_scene->dev_passes), sizeof(float*));

    d_q.memcpy(pass_result, dev_passes + n, rd.pars.width * rd.pars.height * 4 * sizeof(float));

    BOOST_LOG_TRIVIAL(debug) << "Pass retrieved!";

    return pass_result;
}


Manager::Manager(CommandManager* _cm) {
    this->cm = _cm;
}

void DenoiseManager::denoise(int width, int height, float* raw, float* result) {

    BOOST_LOG_TRIVIAL(trace) << "DenoiseManager::denoise";

    oidnCommitDevice(device);

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
    if (oidnGetDeviceError(device, &errorMessage) != OIDN_ERROR_NONE) {
        BOOST_LOG_TRIVIAL(error) << errorMessage;
    }

    // Cleanup
    oidnReleaseFilter(filter);
    oidnReleaseDevice(device);
}

