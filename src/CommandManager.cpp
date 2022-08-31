#include "CommandManager.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "libs/stb_image_write.h"

CommandManager::CommandManager() {
    im = std::make_shared<InputManager>(this);
    rm = std::make_shared<RenderingManager>(this);
    dm = std::make_shared<DenoiseManager>(this);
    sm = std::make_shared<SceneManager>(this);
}

void CommandManager::get_pass(std::string& pass) {

    BOOST_LOG_TRIVIAL(trace) << "CommandManager::get_pass(" << pass << ")";

    Message pass_data_msg;

    pass_data_msg.msg = "beauty_data";
    pass_data_msg.type = Message::TYPE_BUFFER;
    pass_data_msg.data_size = rm->rd.pars.width * rm->rd.pars.height * 4 * sizeof(float);
    pass_data_msg.data_type = Message::DATA_TYPE_FLOAT;
    pass_data_msg.data = rm->get_pass(pass);

    im->write_message(pass_data_msg);
}

void CommandManager::get_render_info() {

    RenderingManager::RenderInfo render_info = rm->get_render_info();

    Message render_info_msg;

    boost::json::object json_info;

    json_info["samples"] = render_info.samples;

    render_info_msg.msg = boost::json::serialize(json_info);
    render_info_msg.type = Message::TYPE_RENDER_INFO;
    im->write_message(render_info_msg);
}

void CommandManager::load_texture(Texture texture) {
    BOOST_LOG_TRIVIAL(trace) << "CommandManager::load_texture(" << texture.name << ")";
    sm->scene.addTexture(texture);
    sm->scene.pair_textures();

    im->write_message(Message::OK());
}

void CommandManager::save_pass(std::string& pass, std::string& path) {

    printf("Saving file %s...\n", path.c_str());

    float* pass_data = rm->get_pass(pass);

    unsigned char* saveBuffer =
        new unsigned char[rm->rd.pars.width * rm->rd.pars.height * 4];

    for (int i = 0; i < rm->rd.pars.width * rm->rd.pars.height * 4; i++) {
        saveBuffer[i] = pow((double)pass_data[i], (1.0 / 2.2)) * 255;
    }

    stbi_write_png(path.c_str(), rm->rd.pars.width, rm->rd.pars.height, 4,
        saveBuffer, rm->rd.pars.width * 4);

    delete[](saveBuffer);

    printf("Saved!\n");
}

Vector3 parse_vector3(boost::json::object json) {
    return Vector3(json["r"].as_double(), json["g"].as_double(), json["b"].as_double());
}

void CommandManager::load_material_from_json(boost::json::object json_mat) {

    BOOST_LOG_TRIVIAL(trace) << "CommandManager::load_material_from_json()" << json_mat;

    Material mtl;

    if (json_mat.if_contains("name"))
        mtl.name = json_mat["name"].as_string();

    if (json_mat.if_contains("albedo"))
        mtl.albedo = parse_vector3(json_mat["albedo"].as_object());

    if (json_mat.if_contains("emission"))
        mtl.emission = parse_vector3(json_mat["emission"].as_object());

    if (json_mat.if_contains("roughness"))
        mtl.roughness = json_mat["roughness"].as_double();

    if (json_mat.if_contains("metalness"))
        mtl.metallic = json_mat["metalness"].as_double();

    if (json_mat.if_contains("specular"))
        mtl.specular = json_mat["specular"].as_double();


    if (json_mat.if_contains("albedo_map"))
        mtl.albedo_map = json_mat["albedo_map"].as_string();

    if (json_mat.if_contains("emission_map"))
        mtl.emission_map = json_mat["emission_map"].as_string();

    if (json_mat.if_contains("roughness_map"))
        mtl.roughness_map = json_mat["roughness_map"].as_string();

    if (json_mat.if_contains("metallic_map"))
        mtl.metallic_map = json_mat["metallic_map"].as_string();

    if (json_mat.if_contains("normal_map"))
        mtl.normal_map = json_mat["normal_map"].as_string();

    if (json_mat.if_contains("opacity_map"))
        mtl.opacity_map = json_mat["opacity_map"].as_string();

    BOOST_LOG_TRIVIAL(debug) << "Material parsed: " << mtl.name <<
        " Albedo: " << mtl.albedo.x << ", " << mtl.albedo.y << ", " << mtl.albedo.z << ", " <<
        " Emission: " << mtl.emission.x << ", " << mtl.emission.y << ", " << mtl.emission.z << ", " << 
        " Metalness: " << mtl.metallic <<
        " Roughness: " << mtl.roughness <<
        " Specular: " << mtl.specular <<
        " Albedo Path: " << mtl.albedo_map <<
        " Normal Path: " << mtl.albedo_map;

    sm->scene.addMaterial(mtl);
    sm->scene.pair_materials();
    sm->scene.pair_textures();

    im->write_message(Message::OK());
}

void CommandManager::load_scene_from_obj(std::string& path) {
    BOOST_LOG_TRIVIAL(trace) << "CommandManager::load_scene_from_obj(" << path << ")";
    sm->scene = Scene::loadScene(path);

    im->write_message(Message::OK());
}

void CommandManager::start_render() {
    BOOST_LOG_TRIVIAL(trace) << "CommandManager::start_render";
    rm->start_rendering(&(sm->scene));
}

void CommandManager::run() {
    
    while (1) {

        __asm("");

        while (!command_queue.empty()) {
            std::cout << "processing command " << std::endl;
            command_queue.front()();
            command_queue.pop();
        }
    }

    
}

void CommandManager::init() {
    im_t = std::thread(&InputManager::run_tcp, this->im);
    //cm_t = std::thread(&CommandManager::run, this);
    //wm->run();
    run();
}