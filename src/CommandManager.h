#pragma once

#include <iostream>
#include <thread>
#include <queue>
#include "Managers.h"

class Command {

};

template<class T>
class LoadCommand : public Command {

    T object;
public:
    using LoadCommandType = T;

};

class LoadCameraCommand : public LoadCommand<Camera> {
    static Camera json_to_camera(boost::json::object camera_json);
};


class CommandManager {

public:

    std::shared_ptr<InputManager> im;
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

    void run();
    void init();

};