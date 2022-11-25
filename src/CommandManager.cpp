#include "CommandManager.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

CommandManager::CommandManager() {
    im = std::make_shared<InputManager>(this);
    rm = std::make_shared<RenderingManager>(this);
    dm = std::make_shared<DenoiseManager>(this);
    sm = std::make_shared<SceneManager>(this);
}

void CommandManager::get_pass(std::string& pass) {

    BOOST_LOG_TRIVIAL(trace) << "CommandManager::get_pass(" << pass << ")";

    Message pass_data_msg;

    pass_data_msg.type = Message::Type::DATA;
    pass_data_msg.data_size = rm->rd.pars.width * rm->rd.pars.height * 4 * sizeof(float);
    pass_data_msg.data_format = Message::DataFormat::FLOAT4;
    
    float* raw_pass = rm->get_pass(pass);
    
    //TODO: fix this denoising patch.
    if (rm->get_render_info().samples >= 999) {
        float* denoise_pass = static_cast<float*>(malloc(rm->rd.pars.width * rm->rd.pars.height * sizeof(float) * 4));
        dm->denoise(rm->rd.pars.width, rm->rd.pars.height, raw_pass, denoise_pass);
        for (int i = 3; i < rm->rd.pars.width * rm->rd.pars.height * 4; i+=4) {
            denoise_pass[i] = 1;
        }
        pass_data_msg.data = denoise_pass;
    }
    else {
        pass_data_msg.data = raw_pass;
    }

    im->write_message(pass_data_msg);
}

void CommandManager::get_render_info() {

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

void CommandManager::load_texture(Texture texture) {
    BOOST_LOG_TRIVIAL(trace) << "CommandManager::load_texture(" << texture.name << ")";
    sm->scene.addTexture(texture);
    sm->scene.pair_textures();

    im->write_message(Message::OK());
}

void CommandManager::load_hdri(HDRI hdri) {
    BOOST_LOG_TRIVIAL(trace) << "CommandManager::load_hdri(" << hdri.texture.width << "x" << hdri.texture.height << ")";
    BOOST_LOG_TRIVIAL(info) << hdri.texture.data[0];
    sm->scene.addHDRI(hdri);
    im->write_message(Message::OK());
}

void CommandManager::load_config(RenderParameters rp) {
    BOOST_LOG_TRIVIAL(trace) << "CommandManager::load_config(" << rp.width << ", " << rp.height << ", " << rp.sampleTarget << ")";
    rm->rd.pars = rp;
    //TODO: Pars and scene both have redundant resolution.
    sm->scene.x_res = rp.width;
    sm->scene.y_res = rp.height;
    im->write_message(Message::OK());
}

void CommandManager::load_camera() {
    BOOST_LOG_TRIVIAL(trace) << "CommandManager::load_camera()";
    //sm->scene.camera = camera;
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

Vector3 parse_vector3(boost::json::object json) {
    return Vector3(json["r"].as_double(), json["g"].as_double(), json["b"].as_double());
}

void CommandManager::load_material_from_json(boost::json::object json_mat) {

    BOOST_LOG_TRIVIAL(trace) << "CommandManager::load_material_from_json()" << json_mat;

    Material mtl;

    if (json_mat.if_contains("name")) {
        mtl.name = json_mat["name"].as_string();
    }

    if (json_mat.if_contains("albedo")) {
        Vector3 al = parse_vector3(json_mat["albedo"].as_object());

        //mtl.albedo = Vector3(sycl::pow(al.x, 1/2.2f), sycl::pow(al.y, 1 / 2.2f), sycl::pow(al.z, 1 / 2.2f));
        mtl.albedo = al;
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

    BOOST_LOG_TRIVIAL(debug) << "Material parsed: " << mtl.name <<
        " Albedo: " << mtl.albedo.x << ", " << mtl.albedo.y << ", " << mtl.albedo.z << ", " <<
        " Emission: " << mtl.emission.x << ", " << mtl.emission.y << ", " << mtl.emission.z << ", " << 
        " Metalness: " << mtl.metallic <<
        " Roughness: " << mtl.roughness <<
        " Specular: " << mtl.specular <<
        " Albedo Path: " << mtl.albedo_map <<
        " Normal Path: " << mtl.albedo_map;


    float aspect = sycl::sqrt(1.0 - mtl.anisotropic * 0.9);
    mtl.ax = maxf(0.001, mtl.roughness / aspect);
    mtl.ay = maxf(0.001, mtl.roughness * aspect);

    sm->scene.addMaterial(mtl);
    sm->scene.pair_materials();
    sm->scene.pair_textures();

    im->write_message(Message::OK());
}

void CommandManager::load_objects(std::vector<MeshObject> objects) {
    BOOST_LOG_TRIVIAL(trace) << "CommandManager::load_objects(" << objects.size() << ")";

    for (MeshObject obj : objects) {
        sm->scene.addMeshObject(obj);
    }

    sm->scene.pair_materials();
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
    im->write_message(Message::OK());
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