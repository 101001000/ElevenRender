#pragma once

#include "CommandManager.h"
#include "Window.h"
#include "kernel.h"
#include <functional>

#include <iomanip>
#include <OpenImageDenoise/oidn.hpp>
#include <boost/program_options.hpp>

//Forward declaration
class CommandManager;


class Manager {

public:
    CommandManager* cm;
    Manager(CommandManager* _cm);
};


class SceneManager : Manager {
    
public:
    Scene scene;

    using Manager::Manager;
};

class RenderingManager : Manager {
    
public:

    dev_Scene* dev_scene;
    RenderParameters rp;
    sycl::queue q;

    RenderingManager(CommandManager* _cm);

    void start_rendering(Scene* scene);
    float* get_pass(std::string pass);

};

class InputManager : Manager {

public:
    using Manager::Manager;
    void run();
};


class WindowManager : Manager {

    Window window;
    PixelBuffer pb;

public:

    std::mutex start_window_mutex;
    bool start_window = false;

    WindowManager(CommandManager* _cm);

    void run();
    void start();
    void set_preview_data(float* data);

};

class DenoiseManager : Manager {

    OIDNDevice device = oidnNewDevice(OIDN_DEVICE_TYPE_DEFAULT);

    
public:
    using Manager::Manager;
    void denoise(int width, int height, float* raw, float* result);
};