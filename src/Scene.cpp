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

void Scene::pair_textures() {

	for (int i = 0; i < materials.size(); i++) {

		for (int j = 0; j < textures.size(); j++) {

			if (textures[j].path == materials[i].albedo_map)
				materials[i].albedoTextureID = j;
			if (textures[j].path == materials[i].albedo_map)
				materials[i].emissionTextureID = j;
			if (textures[j].path == materials[i].albedo_map)
				materials[i].roughnessTextureID = j;
			if (textures[j].path == materials[i].albedo_map)
				materials[i].metallicTextureID = j;
			if (textures[j].path == materials[i].albedo_map)
				materials[i].opacityTextureID = j;
			if (textures[j].path == materials[i].albedo_map)
				materials[i].normalTextureID = j;
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

Scene Scene::loadScene(std::string path) {

	ObjLoader objLoader;

	path += "\\";

	printf("Loading scene \n");

	Scene scene = Scene();

	std::ifstream st(path + "scene.json");

	printf("Opening %s", (path + "scene.json").c_str());

	std::string str((std::istreambuf_iterator<char>(st)),
		std::istreambuf_iterator<char>());


	RSJresource scene_json(str);

	printf("Loading camera...\n");

	// Camera
	RSJresource camera_json = scene_json["camera"].as<RSJresource>();
	RSJresource camera_pos_json = camera_json["position"].as<RSJresource>();
	RSJresource camera_rot_json = camera_json["rotation"].as<RSJresource>();

	int xRes = camera_json["xRes"].as<int>();
	int yRes = camera_json["yRes"].as<int>();

	float focalLength = camera_json["focalLength"].as<double>();
	float focusDistance = camera_json["focusDistance"].as<double>();
	float aperture = camera_json["aperture"].as<double>();
	float bokeh = camera_json["bokeh"].as<bool>();

	Vector3 cameraPosition = Vector3(camera_pos_json["x"].as<double>(), camera_pos_json["y"].as<double>(), camera_pos_json["z"].as<double>());
	Vector3 cameraRotation = Vector3(camera_rot_json["x"].as<double>(), camera_rot_json["y"].as<double>(), camera_rot_json["z"].as<double>());

	scene.camera = Camera(xRes, yRes);
	scene.camera.focalLength = focalLength;
	scene.camera.focusDistance = focusDistance;
	scene.camera.aperture = aperture;
	scene.camera.position = cameraPosition;
	scene.camera.rotation = cameraRotation;
	scene.camera.bokeh = bokeh;


	printf("Loading HDRI...\n");

	// HDRI

	RSJresource hdri_json = scene_json["hdri"].as<RSJresource>();

	if (hdri_json["name"].exists()) {
		scene.addHDRI(path + "HDRI\\" + hdri_json["name"].as<std::string>() + ".hdr");
	}
	else if (hdri_json["color"].exists()) {
		Vector3 color = Vector3(hdri_json["color"]["r"].as<double>(), hdri_json["color"]["g"].as<double>(), hdri_json["color"]["b"].as<double>());
		//Vector3 gc_color = Vector3(sycl::pow(color.x, 2.2f), sycl::pow(color.y, 2.2f), sycl::pow(color.z, 2.2f));
		scene.addHDRI(color);
	}


	if (hdri_json["xOffset"].exists())
		scene.hdri.texture.xOffset = hdri_json["xOffset"].as<double>();

	if (hdri_json["yOffset"].exists())
		scene.hdri.texture.xOffset = hdri_json["yOffset"].as<double>();

	printf("Loading objs...\n");

	std::vector<UnloadedMaterial> umtls(0);
	std::vector<MeshObject> objects(0);

	objLoader.loadObjsRapid(path + "scene.obj", objects, umtls);

	//Materials v2
	//std::vector<UnloadedMaterial> umtls = objLoader.loadMtls(path + "scene.mtl");

	/*
	for (int i = 0; i < umtls.size(); i++) {

		

		for (const auto& map : umtls[i].maps) {

			std::string key = map.first;
			std::string mapPath = umtls[i].maps[key];

			CS colorSpace = CS::LINEAR;

			int textureId = scene.textureCount();
			bool dupTex = false;

			for (int j = 0; j < scene.textures.size(); j++) {
				if (scene.textures[j].path == mapPath) {
					textureId = j;
					dupTex = true;
				}
			}

			if (key == "map_Kd") {
				umtls[i].mat.albedoTextureID = textureId;
				colorSpace = CS::sRGB;
			}

			if (key == "map_Ns")
				umtls[i].mat.roughnessTextureID = textureId;

			if (key == "refl")
				umtls[i].mat.metallicTextureID = textureId;

			if (key == "map_Bump")
				umtls[i].mat.normalTextureID = textureId;

			if(!dupTex)
				scene.addTexture(Texture(mapPath, colorSpace));
		}
		
		//scene.addMaterial(umtls[i].mat);
	}
	*/

	for (int i = 0; i < objects.size(); i++) {
		scene.addMeshObject(objects[i]);	
	}

	printf("%d objects loaded.\n", scene.meshObjectCount());

	scene.pair_materials();

	// PointLights

	RSJarray pointLight_json = scene_json["pointLights"].as<RSJarray>();

	for (int i = 0; i < pointLight_json.size(); i++) {

		Vector3 position = pointLight_json[i]["position"].as<Vector3>();
		Vector3 radiance = pointLight_json[i]["radiance"].as<Vector3>();

		scene.addPointLight(PointLight(position, radiance));

		printf("Loaded pointLight pos ");
		position.print();
		printf(" radiance ");
		radiance.print();
		printf("\n");
	}

	return scene;
}
