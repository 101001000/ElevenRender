#include "CommandManager.h"
#include "kernel.h"
#include "sycl.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

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

Texture parse_texturejson(boost::json::object texture_metadata_json, float* data) {

    int width = texture_metadata_json["width"].as_int64();
    int height = texture_metadata_json["height"].as_int64();
    int channels = texture_metadata_json["channels"].as_int64();

    Texture::CS cs;

    if (texture_metadata_json["color_space"].as_string() == "LINEAR") {
        cs = Texture::CS::LINEAR;
    }
    else if (texture_metadata_json["color_space"].as_string() == "sRGB") {
        cs = Texture::CS::sRGB;
    }
    else {
        LOG(warning) << "Texture metadata colorspace not recognized";
    }

    return Texture(texture_metadata_json["name"].as_string().c_str(), width, height, channels, data, Texture::Filter::NO_FILTER, cs);
}



Vector3 parse_vector3(boost::json::object json) {
    return Vector3(json["r"].as_double(), json["g"].as_double(), json["b"].as_double());
}



Material parse_materialjson(boost::json::object json_mat) {

    Material mtl;

    if (json_mat.if_contains("name")) {
        mtl.name = json_mat["name"].as_string();
    }

    if (json_mat.if_contains("albedo")) {
        mtl.albedo = parse_vector3(json_mat["albedo"].as_object());
    }

    if (json_mat.if_contains("emission")) {
        mtl.emission = parse_vector3(json_mat["emission"].as_object());
    }
    if (json_mat.if_contains("roughness")) {
        mtl.roughness = json_mat["roughness"].as_double();
    }
    if (json_mat.if_contains("metalness")) {
        mtl.metallic = json_mat["metalness"].as_double();
    }
    if (json_mat.if_contains("specular")) {
        mtl.specular = json_mat["specular"].as_double();
    }
    if (json_mat.if_contains("opacity")) {
        mtl.opacity = json_mat["opacity"].as_double();
    }
    if (json_mat.if_contains("transmission")) {
        mtl.transmission = json_mat["transmission"].as_double();
    }

    if (json_mat.if_contains("albedo_map")) {
        mtl.albedo_map = json_mat["albedo_map"].as_string();
    }
    if (json_mat.if_contains("emission_map")) {
        mtl.emission_map = json_mat["emission_map"].as_string();
    }
    if (json_mat.if_contains("roughness_map")) {
        mtl.roughness_map = json_mat["roughness_map"].as_string();
    }
    if (json_mat.if_contains("metallic_map")) {
        mtl.metallic_map = json_mat["metallic_map"].as_string();
    }
    if (json_mat.if_contains("normal_map")) {
        mtl.normal_map = json_mat["normal_map"].as_string();
    }
    if (json_mat.if_contains("opacity_map")) {
        mtl.opacity_map = json_mat["opacity_map"].as_string();
    }
    if (json_mat.if_contains("transmission_map")) {
        mtl.transmission_map = json_mat["transmission_map"].as_string();
    }
    if (json_mat.if_contains("albedo_shader_id")) {
        mtl.albedoShaderID = json_mat["albedo_shader_id"].as_int64();
    }

    float aspect = sycl::sqrt(1.0 - mtl.anisotropic * 0.9);
    mtl.ax = maxf(0.001, mtl.roughness / aspect);
    mtl.ay = maxf(0.001, mtl.roughness * aspect);
    return mtl;
}



Camera CameraDiskLoadInputCommand::load() {
    LOG(error) << "Not implemented yet";
}

Camera CameraTCPLoadInputCommand::load() {
    Camera camera;
    try {
        camera = parse_camerajson(msg.get_json_data());
    }
    catch (std::exception const& e) {
        LOG(error) << "Invalid camera format: " << e.what();
    }
    return camera ;
}

Texture TextureDiskLoadInputCommand::load() {
    LOG(error) << "Not implemented yet";
}

Texture TextureTCPLoadInputCommand::load() {
    LOG(trace) << "TextureTCPLoadInputCommand::load";
    Texture texture;
    boost::json::object json_metadata = metadata_msg.get_json_data();
    float* data = data_msg.get_float_data();

    try {
        texture = parse_texturejson(json_metadata, data);
    }
    catch (std::exception const& e) {
        LOG(error) << "Invalid texture format: " << e.what();
    }
    return texture;
}

RenderParameters ConfigDiskLoadInputCommand::load() {
    LOG(error) << "Not implemented yet";
}

RenderParameters ConfigTCPLoadInputCommand::load() {

    boost::json::object json_data = msg.get_json_data();
    RenderParameters rp;

    try {
        rp = RenderParameters(json_data["x_res"].as_int64(),
            json_data["y_res"].as_int64(),
            json_data["sample_target"].as_int64(),
            json_data["denoise"].as_bool(),
            json_data["device"].as_string().c_str(),
            json_data["block_size"].as_int64());
    }
    catch (std::exception const& e) {
        LOG(error) << "Invalid config format: " << e.what();
    }

    return rp;
}

HDRI HdriDiskLoadInputCommand::load() {
    LOG(error) << "Not implemented yet";
}

HDRI HdriTCPLoadInputCommand::load() {

    Texture texture;
    boost::json::object json_metadata = metadata_msg.get_json_data();
    float* data = data_msg.get_float_data();

    try {
        texture = parse_texturejson(json_metadata, data);
        if (mirror_x) texture.mirror_x();
        if (mirror_y) texture.mirror_y();
        texture.pixel_shift(0.5, 0);
    }
    catch (std::exception const& e) {
        LOG(error) << "Invalid config format: " << e.what();
    }
    return HDRI(texture);
}

Material BrdfTCPLoadInputCommand::load() {
    Material material;
    try {
        material = parse_materialjson(msg.get_json_data());
    }
    catch (std::exception const& e) {
        LOG(error) << "Invalid material format: " << e.what();
    }
    return material;
}

Material BrdfDiskLoadInputCommand::load() {
    LOG(error) << "Not implemented yet";
}


std::vector<MeshObject> ObjectsTCPLoadInputCommand::load() {
    LOG(trace) << "ObjectsTCPLoadInputCommand::load(" << recompute_normals << ")";
    std::vector<MeshObject> objects(0);
    ObjLoader objLoader;
    LOG(trace) << "Buffer creating";
    boost::interprocess::bufferstream input_stream(static_cast<char*>(msg.data), msg.data_size);
    LOG(trace) << "Buffer created " << msg.data_size;
    try {
        objLoader.loadObjsRapid(input_stream, std::string_view(static_cast<char*>(mtls_msg.data)), objects, recompute_normals);
    }
    catch (std::exception const& e) {
        LOG(error) << "Invalid .obj file: " << e.what();
    }
    LOG(trace) << "Objects loaded";
    return objects;
}

std::vector<MeshObject> ObjectsDiskLoadInputCommand::load() {
    LOG(trace) << "ObjectsTCPLoadInputCommand::load()";
    std::vector<MeshObject> objects(0);
    ObjLoader objLoader;
    path.erase(remove(path.begin(), path.end(), '\"'), path.end()); // Remove double quotes
    objLoader.loadObjsRapid(path, objects, recompute_normals);
    return objects;
}



CommandManager::CommandManager() {
    LOG(debug) << "CommandManager constructor called";
    LOG(debug) << "RM constructor called";
    rm = std::make_shared<RenderingManager>(this);
    LOG(debug) << "DM constructor called";
    dm = std::make_shared<DenoiseManager>(this);
    LOG(debug) << "SM constructor called";
    sm = std::make_shared<SceneManager>(this);
}

void CommandManager::get_pass(std::string& pass) {

    LOG(trace) << "CommandManager::get_pass(" << pass << ")";

    Message pass_data_msg;

    pass_data_msg.type = Message::Type::DATA;
    pass_data_msg.data_size = rm->rd.pars.width * rm->rd.pars.height * 4 * sizeof(float);
    pass_data_msg.data_format = Message::DataFormat::FLOAT4;
    
    float* raw_pass = rm->get_pass(pass);
    
    LOG(info) << "DENOISE: " << rm->rd.pars.denoise;

    //TODO: fix this denoising patch.
    if (rm->rd.pars.denoise) {
        LOG(debug) << "Denoise started";
        float* denoise_pass = static_cast<float*>(malloc(rm->rd.pars.width * rm->rd.pars.height * sizeof(float) * 4));
        dm->denoise(rm->rd.pars.width, rm->rd.pars.height, raw_pass, denoise_pass);
        for (int i = 3; i < rm->rd.pars.width * rm->rd.pars.height * 4; i+=4) {
            denoise_pass[i] = 1;
        }
        pass_data_msg.data = denoise_pass;
        LOG(debug) << "Denoise ended";
    }
    else {
        pass_data_msg.data = raw_pass;
    }

    im->write_message(pass_data_msg);
}

void CommandManager::get_render_info() {
    LOG(trace) << "CommandManager::get_render_info()";
    RenderingManager::RenderInfo render_info = rm->get_render_info();

    Message render_info_msg;

    boost::json::object json_info;

    json_info["samples"] = render_info.samples;

    render_info_msg.data = (void*)(boost::json::serialize(json_info).c_str());
    render_info_msg.data_format = Message::DataFormat::JSON;
    render_info_msg.data_size = boost::json::serialize(json_info).size();
    render_info_msg.type = Message::Type::DATA;
    im->write_message(render_info_msg);
}

void CommandManager::get_sycl_info() {


    try {

        auto platforms = sycl::platform::get_platforms();
        boost::json::object json_info;
        boost::json::array json_devices;

        for (const auto& platform : platforms) {
            auto devices = platform.get_devices();

            for (auto device : devices) {
                boost::json::object json_device;
                json_device["name"] = device.get_info<sycl::info::device::name>();
                json_device["platform"] = platform.get_info<sycl::info::platform::name>();
                json_device["memory"] = device.get_info<sycl::info::device::global_mem_size>();
                json_device["max_compute_units"] = device.get_info<sycl::info::device::max_compute_units>();
                json_device["is_compatible"] = is_compatible(device);

                auto dt = device.get_info<sycl::info::device::device_type>();

                if (dt == sycl::info::device_type::cpu)
                    json_device["type"] = "cpu";
                if (dt == sycl::info::device_type::gpu)
                    json_device["type"] = "gpu";
                if (dt == sycl::info::device_type::host)
                    json_device["type"] = "host";
                if (dt == sycl::info::device_type::accelerator)
                    json_device["type"] = "accelerator";
                if (dt == sycl::info::device_type::all)
                    json_device["type"] = "all";
                if (dt == sycl::info::device_type::automatic)
                    json_device["type"] = "automatic";
                if (dt == sycl::info::device_type::custom)
                    json_device["type"] = "custom";

                json_devices.push_back(json_device);
            }
        }

        json_info["devices"] = json_devices;

        Message sycl_info_msg;

        std::string serialized_string = boost::json::serialize(json_info);
        char* data = new char[serialized_string.size() + 1]; 
        std::strcpy(data, serialized_string.c_str());

        sycl_info_msg.data = (void*)data;
        sycl_info_msg.data_format = Message::DataFormat::JSON;
        sycl_info_msg.data_size = boost::json::serialize(json_info).size();
        sycl_info_msg.type = Message::Type::DATA;
        im->write_message(sycl_info_msg);
    }
    catch (std::exception const& e) {
        LOG(error) << "Invalid get_sycl_info" << e.what();
    }
}

void CommandManager::load_texture(Texture texture) {
    LOG(trace) << "CommandManager::load_texture(" << texture.name << ")";
    sm->scene.addTexture(texture);
    sm->scene.pair_textures();

    im->write_message(Message::OK());
}

void CommandManager::load_hdri(HDRI hdri) {
    LOG(trace) << "CommandManager::load_hdri(" << hdri.texture.width << "x" << hdri.texture.height << ")";
    LOG(info) << hdri.texture.data[0];
    sm->scene.addHDRI(hdri);
    im->write_message(Message::OK());
}

//TODO: redundant and mismatch
void CommandManager::load_config(RenderParameters rp) {
    LOG(trace) << "CommandManager::load_config(" << rp.width << ", " << rp.height << ", " << rp.sampleTarget << rp.device << ")";
    rm->rd.pars = rp;
    //TODO: Pars and scene both have redundant resolution.
    sm->scene.x_res = rp.width;
    sm->scene.y_res = rp.height;
    im->write_message(Message::OK());
}

void CommandManager::load_camera(Camera camera) {
    LOG(trace) << "CommandManager::load_camera()";
    sm->scene.camera = camera;
    im->write_message(Message::OK());
}


void CommandManager::load_brdf_material(Material material) {
    sm->scene.addMaterial(material);
    sm->scene.pair_materials();
    sm->scene.pair_textures();
    im->write_message(Message::OK());
}

void CommandManager::save_pass(std::string& pass, std::string& path) {

    printf("Saving file %s...\n", path.c_str());

    float* pass_data = rm->get_pass(pass);

    unsigned char* saveBuffer =
        new unsigned char[rm->rd.pars.width * rm->rd.pars.height * 4];

    for (int i = 0; i < rm->rd.pars.width * rm->rd.pars.height * 4; i++) {
        saveBuffer[i] = pow(static_cast<double>(pass_data[i]), (1.0 / 2.2)) * 255;
    }

    stbi_write_png(path.c_str(), rm->rd.pars.width, rm->rd.pars.height, 4,
        saveBuffer, rm->rd.pars.width * 4);

    delete[](saveBuffer);

    printf("Saved!\n");
}


void CommandManager::execute_load_input_command(LoadInputCommand* ic) {

    LOG(trace) << "CommandManager::execute_load_input_command";

    if (dynamic_cast<TextureLoadInputCommand*>(ic) != nullptr) {
        Texture texture = dynamic_cast<TextureLoadInputCommand*>(ic)->load();
        load_texture(texture);
    }
    else if (dynamic_cast<CameraLoadInputCommand*>(ic) != nullptr) {
        Camera camera = dynamic_cast<CameraLoadInputCommand*>(ic)->load();
        load_camera(camera);
    }
    else if (dynamic_cast<ConfigLoadInputCommand*>(ic) != nullptr) {
        RenderParameters config = dynamic_cast<ConfigLoadInputCommand*>(ic)->load();
        load_config(config);
    }
    else if (dynamic_cast<HdriLoadInputCommand*>(ic) != nullptr) {
        HDRI hdri = dynamic_cast<HdriLoadInputCommand*>(ic)->load();
        load_hdri(hdri);
    }
    else if (dynamic_cast<BrdfLoadInputCommand*>(ic) != nullptr) {
        Material material = dynamic_cast<BrdfLoadInputCommand*>(ic)->load();
        load_brdf_material(material);
    }
    else if (dynamic_cast<ObjectsLoadInputCommand*>(ic) != nullptr) {
        std::vector<MeshObject> objects = dynamic_cast<ObjectsLoadInputCommand*>(ic)->load();
        load_objects(objects);
    }
    else {
        LOG(error) << "Error in execute_load_input_command";
    }
}

void CommandManager::execute_input_command(InputCommand* ic) {

    LOG(trace) << "CommandManager::execute_input_command";

    if (dynamic_cast<LoadInputCommand*>(ic) != nullptr) {
        execute_load_input_command(dynamic_cast<LoadInputCommand*>(ic));
    }
    else if (dynamic_cast<ActionInputCommand*>(ic) != nullptr) {

        if (dynamic_cast<StartInputCommand*>(ic) != nullptr) {
            start_render();
        }
        else if (dynamic_cast<GetInfoInputCommand*>(ic) != nullptr) {
            get_render_info();
        }
        else if (dynamic_cast<GetSyclInfoInputCommand*>(ic) != nullptr) {
            get_sycl_info();
        }
        else if (dynamic_cast<GetPassInputCommand*>(ic) != nullptr) {
            get_pass(dynamic_cast<GetPassInputCommand*>(ic)->pass);
        }
        else {
            LOG(error) << "Unrecognized input action command";
        }
           
    }
    else {
        LOG(error) << "Unrecognized input command";
    }
}

void CommandManager::load_objects(std::vector<MeshObject> objects) {
    LOG(trace) << "CommandManager::load_objects(" << objects.size() << ")";

    for (MeshObject obj : objects) {
        sm->scene.addMeshObject(obj);
    }

    sm->scene.pair_materials();
    im->write_message(Message::OK());
}

void CommandManager::start_render() {
    LOG(trace) << "CommandManager::start_render";
    rm->start_rendering(&(sm->scene));
    im->write_message(Message::OK());
}
