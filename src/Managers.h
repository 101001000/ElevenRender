#pragma once

#include "CommandManager.h"
#include "Window.h"
#include "kernel.h"

#include <CL/sycl.hpp>
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


class RenderingManager : Manager {
    
public:

    dev_Scene* dev_scene;
    sycl::queue q;

    RenderingManager(CommandManager* _cm);

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

    bool is_open;

    WindowManager(CommandManager* _cm);

    void start();
    void close();
    void set_preview_data(float* data);
    void run();


};

class DenoiseManager : Manager {

    OIDNDevice device = oidnNewDevice(OIDN_DEVICE_TYPE_DEFAULT);

    
public:
    using Manager::Manager;
    void denoise(int width, int height, float* raw, float* result);
};