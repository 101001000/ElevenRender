#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#pragma clang diagnostic ignored "-Winconsistent-dllimport"
#include <CL/sycl.hpp>
#pragma clang diagnostic pop
#include <conio.h>


#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "BVH.h"
#include "Camera.h"
#include "Definitions.h"
#include "PostProcessing.h"
#include "Ray.hpp"
#include "Scene.hpp"
#include "SceneLoader.hpp"
#include "Texture.hpp"
#include "kernel.h"
#include "CommandManager.h"
#include "libs/stb_image_write.h"


int currentPass = 0;

int keyPress() {
    int c = 0;
    while (1)
    {
        c = 0;

        switch ((c = getch())) {
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
        if (pars.passes[i]) {
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

    // cudaMemGetInfo(&data.freeMemory, &data.totalMemory);
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


/*

int main(int argc, char* argv[]) {

    CommandManager cm;
    cm.init();

    return 0;
}*/



int main(int argc, char* argv[]) {

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


    //oidnCommitDevice(device);

    bool saved = false;

    Scene scene = loadScene(std::string(argv[1]));


    printf("%s\n", argv[1]);
    RenderData data;
    data.pars =
        RenderParameters(scene.camera.xRes, scene.camera.yRes, atoi(argv[2]));

    sycl::queue q{CUDASelector()};
    dev_Scene* dev_scene = sycl::malloc_device<dev_Scene>(1, q);

    startRender(q, data, scene, dev_scene);

    Window window(1920, 1080);

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

    //oidnReleaseDevice(device);

    denoise_thread.join();
    t.join();
    // cudaDeviceReset();

    return 0;
}


