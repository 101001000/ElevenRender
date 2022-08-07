#include "CommandManager.h"

CommandManager::CommandManager() {
    im = std::make_shared<InputManager>(this);
    wm = std::make_shared<WindowManager>(this);
    rm = std::make_shared<RenderingManager>(this);
    dm = std::make_shared<DenoiseManager>(this);
    sm = std::make_shared<SceneManager>(this);
}

void CommandManager::open_window() {
    this->wm->start_window_mutex.lock();
    this->wm->start_window = true;
    this->wm->start_window_mutex.unlock();
}

void CommandManager::change_preview(std::string& pass) {
    float* pass_data = rm->get_pass(pass);

    wm->set_preview_data(pass_data);
}

void CommandManager::load_scene_from_obj(std::string& path) {
    sm->scene = Scene::loadScene(path);
}

void CommandManager::start_render() {
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
    im_t = std::thread(&InputManager::run, this->im);
    cm_t = std::thread(&CommandManager::run, this);
    wm->run();
}