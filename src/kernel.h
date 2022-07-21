#include "Camera.hpp"
#include "Scene.hpp"
#include "HDRI.hpp"
#include <CL/sycl.hpp>

#pragma once

enum Passes {BEAUTY, DENOISE, NORMAL, TANGENT, BITANGENT};

Passes parsePass(std::string pass);

class RngGenerator {

public:
	uint32_t seed = 328;
	RngGenerator(uint32_t _seed);
	float next();
};

struct dev_Scene {
    Camera* camera;

    unsigned int sphereCount;
    unsigned int meshObjectCount;
    unsigned int materialCount;
    unsigned int textureCount;
    unsigned int triCount;
    unsigned int pointLightCount;

    PointLight* pointLights;
    MeshObject* meshObjects;
    Material* materials;
    Texture* textures;
    Tri* tris;
    BVH* bvh;
    HDRI* hdri;

    float dev_passes[PASSES_COUNT * 1920 * 1080 * 4];
	unsigned int dev_samples[1920 * 1080];
	RngGenerator dev_randstate[1920 * 1080];
};

struct RenderParameters {

	unsigned int width, height;
	unsigned int sampleTarget;

	bool passes[PASSES_COUNT];

	RenderParameters(unsigned int width, unsigned int height, unsigned int sampleTarget) : width(width), height(height), sampleTarget(sampleTarget) {
		passes[BEAUTY] = true;
		passes[DENOISE] = true;
		passes[NORMAL] = true;
		passes[TANGENT] = true;
		passes[BITANGENT] = true;
	};
	RenderParameters() : width(1280), height(720), sampleTarget(100) {};
};

struct RenderData {

	RenderParameters pars;

	float* passes[PASSES_COUNT];

	size_t freeMemory = 0;
	size_t totalMemory = 0;

	int pathCount = 0;
	int samples = 0;

	std::chrono::steady_clock::time_point startTime;

	RenderData() {};

	~RenderData() {

	};
};

struct HitData {

    float metallic;
    float roughness;

	float clearcoatGloss;
	float clearcoat;
	float anisotropic;
	float eta;
	float transmission;
	float specular;
	float specularTint;
	float sheenTint;
	float subsurface;
	float sheen;

    Vector3 emission;
    Vector3 albedo;


    Vector3 normal;
    Vector3 tangent;
    Vector3 bitangent;
};



void printPdfMaterial(Material material, int samples);
void printBRDFMaterial(Material material, int samples);
void printHDRISampling(HDRI hdri, int samples);
void calcNormalPass();

int renderCuda(sycl::queue& q, Scene* scene, int sampleTarget);

int renderSetup(sycl::queue& q, Scene* scene, dev_Scene* dev_scene);

int getBuffers(dev_Scene* dev_scene, sycl::queue& q, RenderData& renderData,
               int* pathcountBuffer,
               int size);

int getSamples(dev_Scene* dev_scene, sycl::queue& q);

