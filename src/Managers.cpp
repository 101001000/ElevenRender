#include "CommandManager.h"

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
                cm->change_preview(vm["preview_pass"].as<std::string>());
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
        if (device.get_platform().get_backend() == sycl::backend::cuda) {
            std::cout << " CUDA device found " << std::endl;
            return 1;
        }
        else {
            return -1;
        }
    }
};

RenderingManager::RenderingManager(CommandManager* _cm) : Manager(_cm){
    q = sycl::queue(CUDASelector());
    dev_Scene* dev_scene = sycl::malloc_device<dev_Scene>(1, q);
}


float* RenderingManager::get_pass(std::string pass) {
    
    printf("\nRetrieving pass %d\n", parsePass(pass));

    float* a = new float[1920 * 1080 * 4 * PASSES_COUNT];
    float* pass_result = new float[1920 * 1080 * 4];

    q.memcpy(a, dev_scene->dev_passes, 1920 * 1080 * 4 * PASSES_COUNT).wait();

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

/*



while (!glfwWindowShouldClose(window.window)) {

    PixelBuffer pb;

    pb.width = data.pars.width;
    pb.height = data.pars.height;
    pb.channels = 4;
    pb.data = data.passes[currentPass];

    window.previewBuffer = pb;

    if (data.samples >= data.pars.sampleTarget - 1 && !saved) {
        printf("Saving file %s...\n", argv[3]);

        unsigned char* saveBuffer =
            new unsigned char[data.pars.width * data.pars.height * 4];

        for (int i = 0; i < data.pars.width * data.pars.height * 4; i++) {
            saveBuffer[i] = sycl::pow((double)pb.data[i], (1.0 / 2.2)) * 255;
        }

        stbi_write_png(argv[3], data.pars.width, data.pars.height, 4,
            saveBuffer, data.pars.width * 4);
        saved = true;
        delete (saveBuffer);

        printf("Saved!\n");
    }

    auto t2 = std::chrono::high_resolution_clock::now();

    auto ms_int = std::chrono::duration_cast<std::chrono::milliseconds>(
        t2 - data.startTime);

    printf(
        "\rkPaths/s: %f, %fGB of a total of %fGB used, %d/%d samples. %f "
        "seconds running, %d total paths",
        ((float)data.pathCount / (float)ms_int.count()),
        (float)(data.totalMemory - data.freeMemory) / (1024 * 1024 * 1024),
        (float)data.totalMemory / (1024 * 1024 * 1024), data.samples,
        data.pars.sampleTarget, ((float)(ms_int).count()) / 1000,
        data.pathCount);

    window.renderUpdate();

    std::this_thread::sleep_for(std::chrono::milliseconds(UPDATE_INTERVAL));
}

terminateDenoise = true;
window.stop();

oidnReleaseDevice(device);

denoise_thread.join();
t.join();
// cudaDeviceReset();

return 0;
*/

/*

void RenderingManager::init() {


}


*/

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

