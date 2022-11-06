#pragma once

#include "Camera.h"
#include "Scene.h"
#include "HDRI.h"
#include "sycl.h"

enum Passes {BEAUTY, DENOISE, NORMAL, TANGENT, BITANGENT};

Passes parsePass(std::string pass);

class RngGenerator {

public:

	uint32_t state = 328;

	explicit RngGenerator(uint32_t _seed);
	float next();
};



struct dev_Scene {
    Camera* camera;

    unsigned int meshObjectCount;
    unsigned int materialCount;
    unsigned int textureCount;
    unsigned int triCount;
    unsigned int pointLightCount;

	unsigned int x_res;
	unsigned int y_res;

    PointLight* pointLights;
    MeshObject* meshObjects;
    Material* materials;
    Texture* textures;
    Tri* tris;
    BVH* bvh;
    HDRI* hdri;

    float* dev_passes;
	unsigned int* dev_samples;
	RngGenerator* dev_randstate;

	explicit dev_Scene(Scene* scene);
};

struct RenderParameters {

	unsigned int width, height;
	unsigned int sampleTarget;

	bool passes_enabled[PASSES_COUNT];

	RenderParameters(unsigned int width, unsigned int height, unsigned int sampleTarget) : width(width), height(height), sampleTarget(sampleTarget) {
		passes_enabled[BEAUTY] = true;
		passes_enabled[DENOISE] = true;
		passes_enabled[NORMAL] = true;
		passes_enabled[TANGENT] = true;
		passes_enabled[BITANGENT] = true;
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
	float opacity;
	float ax;
	float ay;

    Vector3 emission;
    Vector3 albedo;

	int triIdx;

	Vector3 position;
    Vector3 normal;
	Vector3 gnormal;
    Vector3 tangent;
    Vector3 bitangent;
};


int renderSetup(sycl::queue& q, Scene* scene, dev_Scene* dev_scene);

int getSamples(dev_Scene* dev_scene, sycl::queue& q);

