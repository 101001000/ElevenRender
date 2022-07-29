#pragma once

#include <iostream>
#include <thread>
#include "Managers.h"

class CommandManager {

public:

    std::shared_ptr<InputManager> im;
    std::shared_ptr<WindowManager> wm;
    std::shared_ptr<RenderingManager> rm;
    std::shared_ptr<DenoiseManager> dm;
    std::shared_ptr<SceneManager> sm;

    std::thread im_t;
    std::thread wm_t;
    std::thread dm_t;
    std::thread cm_t;

    CommandManager();


    // WindowManager commands
    void open_window();

    void change_preview(std::string pass);
    void load_scene_from_obj(std::string pass);

    void run();
    void init();

};