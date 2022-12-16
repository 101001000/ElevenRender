#include "Scene.h"

int Scene::materialCount() { return materials.size(); }
int Scene::textureCount() { return textures.size(); }
int Scene::meshObjectCount() {	return meshObjects.size(); }
int Scene::triCount() { return tris.size(); }
int Scene::pointLightCount() {	return pointLights.size();}

Camera* Scene::getMainCamera() { return &camera; }

Material* Scene::getMaterials() {
	if (materials.size() == 0) { return static_cast<Material*>(nullptr); }
	return materials.data();
}

Tri* Scene::getTris() {
	if (tris.size() == 0) { return static_cast<Tri*>(nullptr); }
	return tris.data();
}

MeshObject* Scene::getMeshObjects() {
	if (meshObjects.size() == 0) { return static_cast<MeshObject*>(nullptr); }
	return meshObjects.data();
}

PointLight* Scene::getPointLights() {
	if (pointLights.size() == 0) { return static_cast<PointLight*>(nullptr); }
	return pointLights.data();
}

Texture* Scene::getTextures() {
	if (textures.size() == 0) { return static_cast<Texture*>(nullptr); }
	return textures.data();
}

void Scene::addPointLight(PointLight pointLight) {	pointLights.push_back(pointLight); }

void Scene::addTexture(Texture texture) {
	std::cout << "ADDING TEXTURE" << std::endl;
	if (textureIDs.count(texture.name) == 0) {
		std::cout << "Texture not in the scene, adding to the scene" << std::endl;
		textureIDs[texture.name] = textures.size();
		textures.push_back(texture);
	}
}
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

void Scene::addHDRI(HDRI _hdri) { hdri = _hdri; }

void Scene::pair_textures() {

	//TODO cleanup with hashmap.
	for (int i = 0; i < materials.size(); i++) {

		for (int j = 0; j < textures.size(); j++) {

			if (textures[j].name == materials[i].albedo_map) {
				materials[i].albedoTextureID = j;
			}
			if (textures[j].name == materials[i].emission_map) {
				materials[i].emissionTextureID = j;
			}
			if (textures[j].name == materials[i].roughness_map) {
				materials[i].roughnessTextureID = j;
			}
			if (textures[j].name == materials[i].metallic_map) {
				materials[i].metallicTextureID = j;
			}
			if (textures[j].name == materials[i].opacity_map) {
				materials[i].opacityTextureID = j;
			}
			if (textures[j].name == materials[i].normal_map) {
				materials[i].normalTextureID = j;
			}
		}
	}

}

void Scene::pair_materials() {

	for (int i = 0; i < meshObjects.size(); i++) {

		meshObjects[i].materialID = 0;

		for (int j = 0; j < materials.size(); j++) {

			if (materials[j].name == meshObjects[i].matName) {
				meshObjects[i].materialID = j;
			}
		}
	}

	printf("Materials paired\n");

}

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
