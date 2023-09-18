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
#include <boost/json.hpp>
#include "Logging.h"

class Scene {

public:

	std::vector<Material> materials;
	std::vector<MeshObject> meshObjects;
	std::vector<Texture> textures;
	std::vector<Tri> tris;
	std::vector<PointLight> pointLights;

	std::map<std::string, unsigned int> textureIDs;

	unsigned int x_res;
	unsigned int y_res;
	unsigned int frame;
	unsigned int fps;

	HDRI hdri;
	Camera camera;

public:

	Scene() { materials.push_back(Material::DefaultMaterial());}

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

	void addHDRI(HDRI hdri);

	void pair_materials();
	void pair_textures();

	BVH* buildBVH();

};
