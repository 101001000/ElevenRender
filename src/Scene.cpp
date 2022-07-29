#include "Scene.h"

int Scene::materialCount() { return materials.size(); }
int Scene::textureCount() { return textures.size(); }
int Scene::meshObjectCount() {	return meshObjects.size(); }
int Scene::triCount() { return tris.size(); }
int Scene::pointLightCount() {	return pointLights.size();}

Camera* Scene::getMainCamera() { return &camera; }

Material* Scene::getMaterials() {
	if (materials.size() == 0) return (Material*)0;
	return materials.data();
}

Tri* Scene::getTris() {
	if (tris.size() == 0) return (Tri*)0;
	return tris.data();
}

MeshObject* Scene::getMeshObjects() {
	if (meshObjects.size() == 0) return (MeshObject*)0;
	return meshObjects.data();
}

PointLight* Scene::getPointLights() {
	if (pointLights.size() == 0) return (PointLight*)0;
	return pointLights.data();
}

Texture* Scene::getTextures() {
	if (textures.size() == 0) return (Texture*)0;
	return textures.data();
}

void Scene::addPointLight(PointLight pointLight) {	pointLights.push_back(pointLight); }
void Scene::addTexture(Texture texture) { textures.push_back(texture); }
void Scene::addMaterial(Material material) { materials.push_back(material); }

void Scene::addMeshObject(MeshObject meshObject) {

	meshObject.objectID = meshObjectCount();

	for (int i = 0; i < meshObject.triCount; i++) {
		meshObject.tris[i].objectID = meshObject.objectID;
		tris.push_back(meshObject.tris[i]);
	}

	meshObjects.push_back(meshObject);

	//Update ptrs
	int sum = 0;

	for (int i = 0; i < meshObjects.size(); i++) {
		meshObjects.at(i).tris = tris.data() + sum;
		sum += meshObjects.at(i).triCount;
	}
}

void Scene::addHDRI(std::string filepath) { hdri = HDRI(filepath); }
void Scene::addHDRI(Vector3 color) { hdri = HDRI(color); }

BVH* Scene::buildBVH() {

	printf("\nBuilding BVH with DEPTH=%d and SAHBINS=%d \n", BVH_DEPTH, BVH_SAHBINS);

	auto t1 = std::chrono::high_resolution_clock::now();

	BVH* bvh = new BVH();

	int* triIndices = new int[triCount()];

	bvh->triIndices = triIndices;

	bvh->build(&tris);

	auto t2 = std::chrono::high_resolution_clock::now();

	auto ms_int = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);

	std::cout << "\nBVH built in  " << ms_int.count() << std::endl;

	return bvh;
}

