#include "CommandManager.h"
#include "Managers.h"

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

            if (vm.count("window")) {
                std::cout << "Opening preview window";
                cm->open_window();
            }

            if (vm.count("preview_pass")) {
                std::cout << "adding preview_pass to the queue";
                std::function <void()> f = std::bind(&CommandManager::change_preview, std::ref(cm), vm["preview_pass"].as<std::string>());
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

    //std::thread(renderSetup, std::ref(q), &scene, dev_scene);

}

RenderingManager::RenderingManager(CommandManager* _cm) : Manager(_cm){
    q = sycl::queue(CUDASelector());
    dev_Scene* dev_scene = sycl::malloc_device<dev_Scene>(1, q);
}


float* RenderingManager::get_pass(std::string pass) {
    
    //TODO just get one pass

    printf("\nRetrieving pass %d\n", parsePass(pass));

    float* dev_passes;

    float* a = new float[rp.width * rp.height * 4 * PASSES_COUNT];
    float* pass_result = new float[rp.width * rp.height * 4];

    q.memcpy(&dev_passes, &(dev_scene->dev_passes), sizeof(float*)).wait();
    q.memcpy(a, dev_passes, rp.width * rp.height * 4 * PASSES_COUNT).wait();

    for (int j = 0; j < rp.width * rp.height; j++) {
        int n = parsePass(pass) * rp.width * rp.height * 4;
        pass_result[j * 4 + 0] = a[n + j * 4 + 0];
        pass_result[j * 4 + 1] = a[n + j * 4 + 1];
        pass_result[j * 4 + 2] = a[n + j * 4 + 2];
        pass_result[j * 4 + 3] = a[n + j * 4 + 3];
    }

    delete[] a;

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

