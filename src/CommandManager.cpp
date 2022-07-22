#include "CommandManager.h"

CommandManager::CommandManager() {
    im = std::make_shared<InputManager>(this);
    wm = std::make_shared<WindowManager>(this);
    rm = std::make_shared<RenderingManager>(this);
    dm = std::make_shared<DenoiseManager>(this);}

void CommandManager::open_window() {
    this->wm->start_window_mutex.lock();
    this->wm->start_window = true;
    this->wm->start_window_mutex.unlock();
}

void CommandManager::change_preview(std::string pass) {
    wm->set_preview_data(rm->get_pass(pass));
}

void CommandManager::run() {
    
}

void CommandManager::init() {
    im_t = std::thread(&InputManager::run, this->im);
    wm->run();
}