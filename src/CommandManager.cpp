#include "CommandManager.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "libs/stb_image_write.h"

CommandManager::CommandManager() {
    im = std::make_shared<InputManager>(this);
    //wm = std::make_shared<WindowManager>(this);
    rm = std::make_shared<RenderingManager>(this);
    dm = std::make_shared<DenoiseManager>(this);
    sm = std::make_shared<SceneManager>(this);
}


/*
void CommandManager::open_window() {
    this->wm->start_window_mutex.lock();
    this->wm->start_window = true;
    this->wm->start_window_mutex.unlock();
}

void CommandManager::change_preview(std::string& pass) {
    float* pass_data = rm->get_pass(pass);

    wm->set_preview_data(pass_data);
}
*/


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


void CommandManager::load_scene_from_obj(std::string& path) {
    sm->scene = Scene::loadScene(path);

    Message ok_message;

    ok_message.msg = "ok";
    ok_message.type = Message::TYPE_STATUS;

    im->write_message(ok_message);
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