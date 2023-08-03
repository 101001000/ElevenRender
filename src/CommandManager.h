#pragma once

#include <iostream>
#include <thread>
#include <queue>
#include "Managers.h"
#include "Texture.h"
#include "Camera.h"
#include "TCPInterface.h"

class InputCommand {
public:
    virtual ~InputCommand() {}
};

class ActionInputCommand : public InputCommand {

};

class LoadInputCommand : public InputCommand {
public:
    virtual ~LoadInputCommand() {}
};

class TextureLoadInputCommand : public LoadInputCommand {
public:
    virtual ~TextureLoadInputCommand() {}
    virtual Texture load() = 0;
};

class CameraLoadInputCommand : public LoadInputCommand {
public:
    virtual ~CameraLoadInputCommand() {}
    virtual Camera load() = 0;
};

class CameraDiskLoadInputCommand : public CameraLoadInputCommand {
public:
    std::string path;
    CameraDiskLoadInputCommand(std::string _path) : path(_path) {}
    Camera load() override;
};

class CameraTCPLoadInputCommand : public CameraLoadInputCommand {
public:
    Message msg;
    CameraTCPLoadInputCommand(Message _msg) : msg(_msg) {}
    Camera load() override;
};

class TextureTCPLoadInputCommand : public TextureLoadInputCommand {
public:
    Message msg;
    TextureTCPLoadInputCommand(Message _msg) : msg(_msg) {}
    Texture load() override;
};

class TextureDiskLoadInputCommand : public TextureLoadInputCommand {
public:
    std::string path;
    TextureDiskLoadInputCommand(std::string _path) : path(_path) {}
    Texture load() override;
};






class CommandManager {

public:

    //std::shared_ptr<InputManager> im;
    TCPInterface* im;
    std::shared_ptr<RenderingManager> rm;
    std::shared_ptr<DenoiseManager> dm;
    std::shared_ptr<SceneManager> sm;

    std::thread im_t;
    std::thread cm_t;

    bool stop_command = false;


    std::queue <std::function<void()>> command_queue;

    CommandManager();

    void load_texture(Texture texture);
    void get_render_info();
    void load_material_from_json(boost::json::object json_mat);
    void save_pass(std::string& pass, std::string& path);
    void get_pass(std::string& pass);
    void start_render();
    void stop_render();
    void load_config(RenderParameters rp);
    void load_camera(Camera camera);
    void load_objects(std::vector<MeshObject> objects);
    void load_hdri(HDRI hdri);

    void execute_input_command(InputCommand* ic);
    void execute_load_input_command(LoadInputCommand* ic);

    void run();
    void init();

};