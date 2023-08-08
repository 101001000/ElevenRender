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
public:
    virtual ~ActionInputCommand() {}
};

class StartInputCommand : public ActionInputCommand {
};

class GetInfoInputCommand : public ActionInputCommand {
};

class GetPassInputCommand : public ActionInputCommand {
public:
    std::string pass;
    GetPassInputCommand(std::string _pass) : pass(_pass) {}
};

class LoadInputCommand : public InputCommand {
public:
    virtual ~LoadInputCommand() {}
};

class ConfigLoadInputCommand : public LoadInputCommand {
public:
    virtual ~ConfigLoadInputCommand() {}
    virtual RenderParameters load() = 0;
};

class ConfigTCPLoadInputCommand : public ConfigLoadInputCommand {
public:
    Message msg;
    ConfigTCPLoadInputCommand(Message _msg) : msg(_msg) {}
    RenderParameters load() override;
};

class ConfigDiskLoadInputCommand : public ConfigLoadInputCommand {
public:
    std::string path;
    ConfigDiskLoadInputCommand(std::string _path) : path(_path) {}
    RenderParameters load() override;
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

class TextureLoadInputCommand : public LoadInputCommand {
public:
    virtual ~TextureLoadInputCommand() {}
    virtual Texture load() = 0;
};

class TextureTCPLoadInputCommand : public TextureLoadInputCommand {
public:
    Message metadata_msg;
    Message data_msg;
    TextureTCPLoadInputCommand(Message _metadata_msg, Message _data_msg) : metadata_msg(_metadata_msg), data_msg(_data_msg) {}
    Texture load() override;
};

class TextureDiskLoadInputCommand : public TextureLoadInputCommand {
public:
    std::string path;
    TextureDiskLoadInputCommand(std::string _path) : path(_path) {}
    Texture load() override;
};

class HdriLoadInputCommand : public LoadInputCommand {
public:
    virtual ~HdriLoadInputCommand() {}
    virtual HDRI load() = 0;
};

class HdriTCPLoadInputCommand : public HdriLoadInputCommand {
public:
    Message metadata_msg, data_msg;
    bool mirror_x, mirror_y;
    HdriTCPLoadInputCommand(Message _metadata_msg, Message _data_msg, bool _mirror_x, bool _mirror_y) : metadata_msg(_metadata_msg), data_msg(_data_msg), mirror_x(_mirror_x), mirror_y(_mirror_y) {}
    HDRI load() override;
};

class HdriDiskLoadInputCommand : public HdriLoadInputCommand {
public:
    std::string path;
    HdriDiskLoadInputCommand(std::string _path) : path(_path) {}
    HDRI load() override;
};

class BrdfLoadInputCommand : public LoadInputCommand {
public:
    virtual ~BrdfLoadInputCommand() {}
    virtual Material load() = 0;
};

class BrdfTCPLoadInputCommand : public BrdfLoadInputCommand {
public:
    Message msg;
    BrdfTCPLoadInputCommand(Message _msg) : msg(_msg) {}
    Material load() override;
};

class BrdfDiskLoadInputCommand : public BrdfLoadInputCommand {
public:
    std::string path;
    BrdfDiskLoadInputCommand(std::string _path) : path(_path) {}
    Material load() override;
};

class ObjectsLoadInputCommand : public LoadInputCommand {
public:
    bool recompute_normals;
    ObjectsLoadInputCommand(bool _recompute_normals) : recompute_normals(_recompute_normals) {}
    virtual ~ObjectsLoadInputCommand() {}
    virtual std::vector<MeshObject> load() = 0;
};

class ObjectsTCPLoadInputCommand : public ObjectsLoadInputCommand {
public:
    Message msg;
    ObjectsTCPLoadInputCommand(Message _msg, bool _recompute_normals) : ObjectsLoadInputCommand(_recompute_normals), msg(_msg) {}
    std::vector<MeshObject> load() override;
};

class ObjectsDiskLoadInputCommand : public ObjectsLoadInputCommand {
public:
    std::string path;
    ObjectsDiskLoadInputCommand(std::string _path, bool _recompute_normals) : ObjectsLoadInputCommand(_recompute_normals), path(_path) {}
    std::vector<MeshObject> load() override;
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
    void load_brdf_material(Material material);
    void save_pass(std::string& pass, std::string& path);
    void get_pass(std::string& pass);
    void start_render();
    void load_config(RenderParameters rp);
    void load_camera(Camera camera);
    void load_objects(std::vector<MeshObject> objects);
    void load_hdri(HDRI hdri);

    void execute_input_command(InputCommand* ic);
    void execute_load_input_command(LoadInputCommand* ic);
};