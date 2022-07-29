#pragma once

#include <thread>         
#include <chrono>    
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <fstream>
#include <map>

#include "Material.h"
#include "Tri.h"
#include "Camera.h"
#include "BVH.h"
#include "PointLight.h"
#include "HDRI.h"
#include "ObjLoader.h"

class Scene {

public:

	std::vector<Material> materials;
	std::vector<MeshObject> meshObjects;
	std::vector<Texture> textures;
	std::vector<Tri> tris;
	std::vector<PointLight> pointLights;

	HDRI hdri;
	Camera camera;

public:

	Scene() {}

	int materialCount();
	int textureCount();
	int meshObjectCount();
	int triCount();
	int pointLightCount();

	Camera* getMainCamera();

	Material* getMaterials();

	Tri* getTris();

	MeshObject* getMeshObjects();

	PointLight* getPointLights();

	Texture* getTextures();

	void addPointLight(PointLight pointLight);
	void addTexture(Texture texture);
	void addMaterial(Material material);

	void addMeshObject(MeshObject meshObject);

	void addHDRI(std::string filepath);
	void addHDRI(Vector3 color);

	BVH* buildBVH();



};
