
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include "sycl.h"
#include <conio.h>

#include "Logging.h"



#include "BVH.h"
#include "Camera.h"
#include "Definitions.h"
#include "PostProcessing.h"
#include "Ray.h"
#include "Scene.h"
#include "Texture.hpp"
#include "kernel.h"
#include "CommandManager.h"



int currentPass = 0;

int keyPress() {
    int c = 0;
    while (1)
    {
        c = 0;

        switch ((c = _getch())) {
        case 75:
            std::cout << std::endl << "Left" << std::endl;  // key left
            currentPass--;
            if (currentPass < 0)
                currentPass = PASSES_COUNT - 1;
            currentPass %= PASSES_COUNT;
            break;
        case 77:
            std::cout << std::endl << "Right" << std::endl;  // key right
            currentPass++;
            currentPass %= PASSES_COUNT;
            break;
        }
    }

    return 0;
}

std::thread t;
std::thread t_key;
std::thread denoise_thread;


void startRender(sycl::queue& q, RenderData& data, Scene& scene, dev_Scene* devScene) {
    RenderParameters pars = data.pars;

    for (int i = 0; i < PASSES_COUNT; i++) {
        if (pars.passes_enabled[i]) {
            data.passes[i] = new float[pars.width * pars.height * 4];
            memset(data.passes[i], 0,
                   pars.width * pars.height * 4 * sizeof(float));
        }
    }

    t = std::thread(renderSetup, std::ref(q), &scene, devScene);
    t_key = std::thread(keyPress);

    data.startTime = std::chrono::high_resolution_clock::now();
}

void getRenderData(dev_Scene* dev_scene, sycl::queue& q, RenderData& data) {

    int width = data.pars.width;
    int height = data.pars.height;

    int* pathCountBuffer = new int[width * height];

    getBuffers(dev_scene, q, data, pathCountBuffer, width * height);

    clampPixels(data.passes[BEAUTY], width, height);

    // applysRGB(data.passes[BEAUTY], width, height);

    data.samples = getSamples(dev_scene, q);
    data.pathCount = 0;

    /*

    for (int i = 0; i < width * height; i++)
        data.pathCount += pathCountBuffer[i];

    delete (pathCountBuffer);*/
}


using boost::asio::ip::tcp;

float data[1920 * 1080 * 4];

/*

void session(boost::asio::ip::tcp::socket sock)
{
    try
    {
        for (;;)
        {

            boost::system::error_code error;
            boost::asio::read(sock, boost::asio::buffer(data), error);

            if (error == boost::asio::error::eof)
                break; // Connection closed cleanly by peer.
            else if (error)
                throw boost::system::system_error(error); // Some other error.

            boost::asio::write(sock, boost::asio::buffer("ok", 3));
        }
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception in thread: " << e.what() << "\n";
    }
}
*/

int standalone() {

    /*


    for (int i = 0; i < 1920 * 1080; i++) {
        data[i * 4 + 0] = 0;
        data[i * 4 + 1] = 0;
        data[i * 4 + 2] = 0;
        data[i * 4 + 3] = 1;
    }

    boost::asio::io_context io_context;

    tcp::acceptor a(io_context, tcp::endpoint(tcp::v4(), 5557));
    std::thread(session, a.accept()).detach();

    Window window(1920, 1080);

    window.init();

    printf("Entering loop");

    while (!glfwWindowShouldClose(window.window)) {

        PixelBuffer pb;

        pb.width = 1920;
        pb.height = 1080;

        pb.channels = 4;
        pb.data = data;

        window.previewBuffer = pb;
        window.renderUpdate();

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    window.stop();*/
    return 0;
}

int backend() {
    
    std::cout << "Awaiting for a connection" << std::endl;

    boost::asio::io_context io_context;

    tcp::acceptor a(io_context, tcp::endpoint(tcp::v4(), 5557));

    tcp::socket sock = a.accept();

    std::cout << "Connected!" << std::endl;

    float* reddata = new float[1920 * 1080 * 4];
    float* greendata = new float[1920 * 1080 * 4];
    float* bluedata = new float[1920 * 1080 * 4];

    for (int i = 0; i < 1920 * 1080; i++) {
        reddata[i * 4 + 0] = 1;
        reddata[i * 4 + 1] = 0;
        reddata[i * 4 + 2] = 0;
        reddata[i * 4 + 3] = 1;
    }

    for (int i = 0; i < 1920 * 1080; i++) {
        greendata[i * 4 + 0] = 0;
        greendata[i * 4 + 1] = 1;
        greendata[i * 4 + 2] = 0;
        greendata[i * 4 + 3] = 1;
    }

    for (int i = 0; i < 1920 * 1080; i++) {
        bluedata[i * 4 + 0] = 0;
        bluedata[i * 4 + 1] = 0;
        bluedata[i * 4 + 2] = 1;
        bluedata[i * 4 + 3] = 1;
    }

    for (;;) {

        std::cout << "Enter message: ";

        char request[1024];
        std::cin.getline(request, 1024);


        if (strcmp(request, "red") == 0) {
            std::cout << "ENVIANDO ROJO" << std::endl;
            boost::asio::write(sock, boost::asio::buffer(reddata, 1920 * 1080 * 4 * sizeof(float)));
        }

        if (strcmp(request, "green") == 0) {
            std::cout << "ENVIANDO verde" << std::endl;
            boost::asio::write(sock, boost::asio::buffer(greendata, 1920 * 1080 * 4 * sizeof(float)));
        }

        if (strcmp(request, "blue") == 0) {
            std::cout << "ENVIANDO azul" << std::endl;
            boost::asio::write(sock, boost::asio::buffer(bluedata, 1920 * 1080 * 4 * sizeof(float)));
        }


        char reply[1024];
        size_t reply_length = boost::asio::read(sock,
            boost::asio::buffer(reply, 3));
        std::cout << "Reply is: ";
        std::cout.write(reply, reply_length);
        std::cout << "\n";

    }
 
    return 0;
}

void coloring_formatter(boost::log::record_view const& rec, boost::log::formatting_ostream& strm)
{
    auto severity = rec[boost::log::trivial::severity];
    if (severity)
    {
        // Set the color
        switch (severity.get())
        {
        case boost::log::trivial::severity_level::info:
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
            break;
        case boost::log::trivial::severity_level::warning:
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 14);
            break;
        case boost::log::trivial::severity_level::error:
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 12);
            break;
        case boost::log::trivial::severity_level::fatal:
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 4);
            break;
        default:
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 8);
            break;
        }
    }

    strm << rec[a_timestamp] << " - [" << rec[a_thread_id] << "] [" << rec[boost::log::trivial::severity] << "]: " << rec[boost::log::expressions::smessage];
}





int main(int argc, char* argv[]) {

    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 9);

    typedef boost::log::sinks::synchronous_sink< boost::log::sinks::text_ostream_backend > text_sink;
    boost::shared_ptr< text_sink > sink = boost::make_shared< text_sink >();

    boost::shared_ptr< std::ostream > stream(&std::clog, boost::null_deleter());
    sink->locked_backend()->add_stream(stream);
    sink->set_formatter(&coloring_formatter);


    // Register the sink in the logging core
    boost::log::core::get()->add_sink(sink);

    boost::log::add_common_attributes();

    //boost::log::add_console_log(std::cout, boost::log::keywords::format = ">> %Message%");

    if (argc > 1) {
        if (strcmp(argv[1], "standalone") == 0) {
            BOOST_LOG_TRIVIAL(info) << "Selected standalone mode";
            standalone();
        }
        else {

            BOOST_LOG_TRIVIAL(info) << "Selected backend mode";
  
            CommandManager cm;
            cm.init();

            //backend();
        }
    }

    return 0;
}

/*


int main1(int argc, char* argv[]) {

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


    //oidnCommitDevice(device);

    bool saved = false;

    Scene scene = Scene::loadScene(std::string(argv[1]));

    printf("%s\n", argv[1]);
    RenderData data;
    data.pars =
        RenderParameters(scene.camera.xRes, scene.camera.yRes, atoi(argv[2]));

    sycl::queue q{CUDASelector()};
    dev_Scene* dev_scene = sycl::malloc_device<dev_Scene>(1, q);

    startRender(q, data, scene, dev_scene);

    Window window(scene.camera.xRes, scene.camera.yRes);

    window.init();

    bool terminateDenoise = false;

    //denoise_thread = std::thread(denoise, data, &terminateDenoise);

    printf("Entering loop");

    while (!glfwWindowShouldClose(window.window)) {

        getRenderData(dev_scene, q, data);

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
                saveBuffer[i] = pb.data[i]; //sycl::pow((double)pb.data[i], (1.0 / 2.2)) * 255;
            }

            stbi_write_png(argv[3], data.pars.width, data.pars.height, 4,
                           saveBuffer, data.pars.width * 4);
            saved = true;
            delete[] (saveBuffer);

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

    //oidnReleaseDevice(device);

    denoise_thread.join();
    t.join();
    // cudaDeviceReset();

    return 0;
}


*/