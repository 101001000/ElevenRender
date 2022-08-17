
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

void coloring_formatter(boost::log::record_view const& rec, boost::log::formatting_ostream& strm)
{
    auto severity = rec[boost::log::trivial::severity];
    if (severity)
    {
        // Set the color
        switch (severity.get())
        {
        case boost::log::trivial::severity_level::info:
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 10);
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


    CommandManager cm;
    cm.init();

    return 0;
}
