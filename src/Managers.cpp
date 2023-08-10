#include "CommandManager.h"
#include "Managers.h"
#include "kernel.h"


Message Message::json2header(boost::json::object json) {
    LOG(trace) << "in Message::parse_json()";

    Message msg;
    msg.type = Message::str2type(json["type"].as_string().c_str());
    msg.data_size = json["data_size"].as_int64();
    msg.data_format = str2data_format(json["data_format"].as_string().c_str());


    LOG(trace) << "out Message::parse_json()";
    return msg;
}


Message::Message() {
    type = Type::NONE;
    data_format = DataFormat::NONE;
    data_size = 0;
    data = nullptr;
}

std::string Message::to_string() {
    std::string str = "HEADER: Type: " + Message::type2str(type) + ", Data Format: " + Message::data_format2str(data_format) + ", Data Size: " + std::to_string(data_size) + "\n";

    if (data_format == DataFormat::JSON) {
        str += "BODY: " + boost::json::serialize(get_json_data());
    }

    if (data_format == DataFormat::STRING) {
        str += "BODY: " + get_string_data();
    }
    
    return str;

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

    Message::Type type = Message::Type::NONE;

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

    Message::DataFormat format = Message::DataFormat::NONE;

    if (str == "none") {
        format = Message::DataFormat::NONE;
    }
    else if (str == "float3") {
        format = Message::DataFormat::FLOAT3;
    }
    else if (str == "float4") {
        format = Message::DataFormat::FLOAT4;
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
    LOG(trace) << "Message::get_json_data()";

    if(data_format != DataFormat::JSON)
        LOG(warning) << "Trying to extract invalid json data";

    boost::json::value json;

    try {
        json = boost::json::parse(static_cast<char*>(data));
    }
    catch (std::exception const& e) {
        LOG(error) << e.what();
    }

    return json.as_object();
}

float* Message::get_float_data() {
    LOG(trace) << "Message::get_float_data()";

    if (data_format != DataFormat::FLOAT3 && data_format !=  DataFormat::FLOAT4)
        LOG(warning) << "Trying to extract invalid float data";

    return (static_cast<float*>(data));
}

std::string Message::get_string_data() {
    LOG(trace) << "Message::get_string_data()";

    if (data_format != DataFormat::STRING)
        LOG(warning) << "Trying to extract invalid string data";

    return std::string(static_cast<const char*>(data));
}


boost::json::object Message::msg2json_header(Message msg) {

    LOG(trace) << "Message::parse_message()";
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

class NameSelector : public sycl::device_selector {
public:
    std::string name;

    NameSelector(std::string _name) : name(_name) {};

    int operator()(const sycl::device& device) const override {
        if (device.get_info<sycl::info::device::name>() == name) {
            return 1;
        }
        else {
            return -1;
        }
    }
};


RenderingManager::RenderInfo RenderingManager::get_render_info() {

    LOG(debug) << "Getting render info";

    float* dev_samples;
    unsigned int sample_count = 0;

    d_q.memcpy(&dev_samples, &(dev_scene->dev_samples), sizeof(unsigned int*));

    d_q.memcpy(&sample_count, dev_samples, 1 * sizeof(unsigned int));

    RenderInfo render_info;
    render_info.samples = sample_count;

    LOG(debug) << "Rendering info retrieved";

    return render_info;
}

void RenderingManager::start_rendering(Scene* scene) {

    LOG(trace) << "RenderingManager::start_rendering()";

    try {
        k_q = sycl::queue(NameSelector(rd.pars.device));
        d_q = sycl::queue(NameSelector(rd.pars.device));
    }
    catch (std::exception const& e) {
        LOG(error) << e.what();
    }

    sycl::device device = k_q.get_device();

    LOG(info) << "Device selected: " << device.get_info<sycl::info::device::name>();

    auto work_item_dim = device.get_info<sycl::info::device::max_work_item_dimensions>();
    auto work_item_size = device.get_info<sycl::info::device::max_work_item_sizes>();
    auto work_item_group_size = device.get_info<sycl::info::device::max_work_group_size>();

    //auto test = sycl::info::device::max_work_item_dimensions

    LOG(info) << "dim " << work_item_dim << " is_x: " << work_item_size[0] << " is_y: " << work_item_size[1] << " is_z: " << work_item_size[2] << " gs: " << work_item_group_size;

    dev_scene = sycl::malloc_device<dev_Scene>(1, k_q);

    for (int i = 0; i < PASSES_COUNT; i++) {

        if (rd.pars.passes_enabled[i]) {
            rd.passes[i] = new float[rd.pars.width * rd.pars.height * 4];
            memset(rd.passes[i], 0,
                rd.pars.width * rd.pars.height * 4 * sizeof(float));
        }
    }

    int BLOCK_SIZE = 8;

    renderSetup(k_q, scene, dev_scene, rd.pars.sampleTarget);
    t_rend = std::thread(kernel_render_enqueue, std::ref(k_q), rd.pars.sampleTarget, BLOCK_SIZE, std::ref(scene), std::ref(dev_scene));

    rd.startTime = std::chrono::high_resolution_clock::now();
    LOG(trace) << "LEAVING RenderingManager::start_rendering()";
}



RenderingManager::RenderingManager(CommandManager* _cm) : Manager(_cm){

    LOG(trace) << "RenderingManager::RenderingManager()";

    //LOG(info) << "Listing all devices: " << sycl::device::get_devices(sycl::info::device_type::all).size();
}


float* RenderingManager::get_pass(std::string pass) {
    
    int n = parsePass(pass) * rd.pars.width * rd.pars.height * 4;

    LOG(debug) << "Retrieving pass: " << pass;

    float* dev_passes;
    float* pass_result = new float[rd.pars.width * rd.pars.height * 4];

    d_q.memcpy(&dev_passes, &(dev_scene->dev_passes), sizeof(float*));

    d_q.memcpy(pass_result, dev_passes + n, rd.pars.width * rd.pars.height * 4 * sizeof(float));

    LOG(debug) << "Pass retrieved!";

    return pass_result;
}


Manager::Manager(CommandManager* _cm) {
    this->cm = _cm;
}

DenoiseManager::DenoiseManager(CommandManager* _cm) : Manager(_cm) {
    device = oidnNewDevice(OIDN_DEVICE_TYPE_DEFAULT);
    oidnCommitDevice(device);
}

DenoiseManager::~DenoiseManager() {
    oidnReleaseDevice(device);
}

void DenoiseManager::denoise(int width, int height, float* raw, float* result) {

    LOG(trace) << "DenoiseManager::denoise";

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
        LOG(error) << errorMessage;
    }

    // Cleanup
    oidnReleaseFilter(filter);
}

