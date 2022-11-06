#include "CommandManager.h"
#include "Managers.h"
#include "kernel.h"


Message Message::json2header(boost::json::object json) {
    BOOST_LOG_TRIVIAL(trace) << "in Message::parse_json()";

    Message msg;
    msg.type = Message::str2type(json["type"].as_string().c_str());
    msg.data_size = json["data_size"].as_int64();
    msg.data_format = str2data_format(json["data_format"].as_string().c_str());


    BOOST_LOG_TRIVIAL(trace) << "out Message::parse_json()";
    return msg;
}


Message::Message() {
    type = Type::NONE;
    data_format = DataFormat::NONE;
    data_size = 0;
    data = nullptr;
}

std::string Message::type2str(Message::Type type) {

    std::string str = "und";

    switch (type) {
    case Message::Type::NONE:
        str = "none";
        break;
    case Message::Type::COMMAND:
        str = "command";
        break;
    case Message::Type::STATUS:
        str = "status";
        break;
    case Message::Type::DATA:
        str = "data";
        break;
    }
    return str;
}

Message::Type Message::str2type(std::string str) {

    Message::Type type;

    if (str == "none") {
        type = Message::Type::NONE;
    }
    else if (str == "command") {
        type = Message::Type::COMMAND;
    }
    else if (str == "status") {
        type = Message::Type::STATUS;
    }
    else if (str == "data") {
        type = Message::Type::DATA;
    }
    return type;
}

std::string Message::data_format2str(Message::DataFormat format) {

    std::string str = "und";

    switch (format) {
    case Message::DataFormat::NONE:
        str = "none";
        break;
    case Message::DataFormat::STRING:
        str = "string";
        break;
    case Message::DataFormat::JSON:
        str = "json";
        break;
    case Message::DataFormat::FLOAT3:
        str = "float3";
        break;
    case Message::DataFormat::FLOAT4:
        str = "float4";
        break;
    }
    return str;
}

Message::DataFormat Message::str2data_format(std::string str) {

    Message::DataFormat format;

    if (str == "none") {
        format = Message::DataFormat::NONE;
    }
    else if (str == "float3") {
        format = Message::DataFormat::FLOAT3;
    }
    else if (str == "float4") {
        format = Message::DataFormat::FLOAT3;
    }
    else if (str == "json") {
        format = Message::DataFormat::JSON;
    }
    else if (str == "string") {
        format = Message::DataFormat::STRING;
    }
    return format;
}



boost::json::object Message::get_json_data() {
    BOOST_LOG_TRIVIAL(trace) << "Message::get_json_data()";
    boost::json::value json;

    try {
        json = boost::json::parse(static_cast<const char*>(data));
    }
    catch (std::exception const& e) {
        BOOST_LOG_TRIVIAL(error) << e.what() << (static_cast<const char*>(data));
    }

    return json.as_object();
}

float* Message::get_float_data() {
    BOOST_LOG_TRIVIAL(trace) << "Message::get_float_data()";
    return static_cast<float*>(data);
}

std::string Message::get_string_data() {
    BOOST_LOG_TRIVIAL(trace) << "Message::get_string_data()";
    return std::string(static_cast<const char*>(data));
}


boost::json::object Message::msg2json_header(Message msg) {

    BOOST_LOG_TRIVIAL(trace) << "Message::parse_message()";
    boost::json::object json;

    json["type"] = Message::type2str(msg.type);
    json["data_format"] = Message::data_format2str(msg.data_format);
    json["data_size"] = msg.data_size;

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

