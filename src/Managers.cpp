#include "CommandManager.h"
#include "Managers.h"
#include "kernel.h"

void InputManager::run() {

    namespace po = boost::program_options;

    po::options_description desc("Allowed options");

    desc.add_options()
        ("help", "produce help message")
        ("window", "open a preview window")
        ("preview_pass", po::value<std::string>(), "preview some pass")
        ("start", "start rendering")
        ("stop", "stop rendering")
        ("load_obj", po::value<std::string>(), "load wavefront obj from file path")
        ("save_pass", po::value<std::vector<std::string>>()->multitoken(), "save rendering pass")
        ("load_config", po::value<std::string>(), "load rendering config from file path")
        ;

    while (1) {

        try {

            std::string sinput;
            std::string s;
            po::variables_map vm;

            std::vector<const char*> argv;

            std::getline(std::cin, sinput);
            std::istringstream iss(sinput);

            argv.push_back("ElevenRender");

            while (iss >> std::quoted(s)) {
                char* c = new char[s.size()];
                strcpy(c, s.c_str());
                argv.push_back(c);
            }

            po::store(po::parse_command_line(argv.size(), argv.data(), desc), vm);
            po::notify(vm);

            if (vm.count("help")) {
                std::cout << desc << "\n";
            }

            /*

            if (vm.count("preview_pass")) {
                std::cout << "adding preview_pass to the queue";
                std::function <void()> f = std::bind(&CommandManager::change_preview, std::ref(cm), vm["preview_pass"].as<std::string>());
                cm->command_queue.push(f);
            }

            */

            if (vm.count("load_obj")) {
                std::cout << "adding load_obj to the queue";
                std::function <void()> f = std::bind(&CommandManager::load_scene_from_obj, std::ref(cm), vm["load_obj"].as<std::string>());
                cm->command_queue.push(f);
            }

            if (vm.count("save_pass")) {
                std::cout << "adding save_pass to the queue";
                std::string pass = vm["save_pass"].as<std::vector<std::string>>()[0];
                std::string path = vm["save_pass"].as<std::vector<std::string>>()[1];
                std::function <void()> f = std::bind(&CommandManager::save_pass, std::ref(cm), pass, path);
                cm->command_queue.push(f);
            }

            if (vm.count("start")) {
                std::cout << "adding start to the queue";
                std::function <void()> f = std::bind(&CommandManager::start_render, std::ref(cm));
                cm->command_queue.push(f);
            }

            for (const char* c : argv) {

                if (strcmp(c, "ElevenRender") != 0) {
                    delete[] c;
                }
            }

        } catch (std::exception const& e) {
            std::cerr << e.what() << "\n";
        }

    }
}

/*

WindowManager::WindowManager(CommandManager* _cm) : Manager(_cm), window(1920, 1080) {

    // PARAMETRIZE PX SIZE FOR THIS MANAGER

    pb.width = 1920;
    pb.height = 1080;
    pb.channels = 4;
    pb.data = new float[1920 * 1080 * 4];

    for (int i = 0; i < 1920 * 1080; i++) {
        pb.data[i * 4 + 0] = 0;
        pb.data[i * 4 + 1] = 0;
        pb.data[i * 4 + 2] = 1;
        pb.data[i * 4 + 3] = 1;
    }

    window.previewBuffer = pb;
};

void WindowManager::run() {

    while (1) {
        if (start_window) {
            start();
            start_window_mutex.lock();
            start_window = false;
            start_window_mutex.unlock();
        }     
        std::this_thread::sleep_for(std::chrono::milliseconds(UPDATE_INTERVAL));
    }
}

void WindowManager::start() {
    window.init();
    while (!glfwWindowShouldClose(window.window)) {
        // TODO make this a reference
        window.previewBuffer = pb;
        window.renderUpdate();
        std::this_thread::sleep_for(std::chrono::milliseconds(UPDATE_INTERVAL));
    }
    window.stop();
}

void WindowManager::set_preview_data(float* data) {
    pb.data = data;
}

*/


class CUDASelector : public sycl::device_selector {
public:
    int operator()(const sycl::device& device) const override {
        if (device.get_platform().get_backend() == sycl::backend::ext_oneapi_cuda) {
            std::cout << " CUDA device found " << std::endl;
            return 1;
        }
        else {
            return -1;
        }
    }
};

void RenderingManager::start_rendering(Scene* scene) {

    rd.pars = RenderParameters(scene->camera.xRes, scene->camera.yRes, 50);

    for (int i = 0; i < PASSES_COUNT; i++) {

        if (rd.pars.passes_enabled[i]) {
            rd.passes[i] = new float[rd.pars.width * rd.pars.height * 4];
            memset(rd.passes[i], 0,
                rd.pars.width * rd.pars.height * 4 * sizeof(float));
        }
    }

    t_rend = std::thread(renderSetup, std::ref(q), std::ref(scene), std::ref(dev_scene));

    //t_key = std::thread(key_press);

    rd.startTime = std::chrono::high_resolution_clock::now();
}

RenderingManager::RenderingManager(CommandManager* _cm) : Manager(_cm){
    q = sycl::queue(CUDASelector());
    dev_scene = sycl::malloc_device<dev_Scene>(1, q);
}


float* RenderingManager::get_pass(std::string pass) {
    
    int n = parsePass(pass) * rd.pars.width * rd.pars.height * 4;

    printf("\nRetrieving pass %d ...\n", parsePass(pass));

    float* dev_passes;
    float* pass_result = new float[rd.pars.width * rd.pars.height * 4];

    q.memcpy(&dev_passes, &(dev_scene->dev_passes), sizeof(float*)).wait();
    q.memcpy(pass_result, dev_passes + n, rd.pars.width * rd.pars.height * 4 * sizeof(float)).wait();

    return pass_result;
}


Manager::Manager(CommandManager* _cm) {
    this->cm = _cm;
}

void DenoiseManager::denoise(int width, int height, float* raw, float* result) {
    OIDNFilter filter =
        oidnNewFilter(device, "RT");  // generic ray tracing filter
    oidnSetSharedFilterImage(filter, "color", raw,
        OIDN_FORMAT_FLOAT3, width, height, 0,
        sizeof(float) * 4, 0);  // beauty
    oidnSetFilter1b(filter, "hdr", true);            // beauty image is HDR
    oidnSetSharedFilterImage(filter, "output", result,
        OIDN_FORMAT_FLOAT3, width, height, 0,
        sizeof(float) * 4, 0);  // denoised beauty

    oidnCommitFilter(filter);
    oidnExecuteFilter(filter);

    const char* errorMessage;
    if (oidnGetDeviceError(device, &errorMessage) != OIDN_ERROR_NONE)
        printf("Error: %s\n", errorMessage);

    // Cleanup
    oidnReleaseFilter(filter);
}

