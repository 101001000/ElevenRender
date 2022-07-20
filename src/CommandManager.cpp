#include "CommandManager.h"

CommandManager::CommandManager() {
    im = std::make_shared<InputManager>(this);
    wm = std::make_shared<WindowManager>(this);
    rm = std::make_shared<RenderingManager>(this);
    dm = std::make_shared<DenoiseManager>(this);}

void CommandManager::open_window() {
    wm->start();
}

void CommandManager::close_window() {
    wm->close();
}

void CommandManager::toggle_window() {
    wm->is_open ? wm->close() : wm->start();
}

void CommandManager::change_preview(std::string pass) {
    //wm->setPreviewData(rm->get_pass(pass));
}

void CommandManager::init() {

    im_t = std::thread(&InputManager::run, this->im);
}