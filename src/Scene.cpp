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

void Scene::addHDRI(std::string filepath) { hdri = HDRI(filepath); }
void Scene::addHDRI(Vector3 color) { hdri = HDRI(color); }

void Scene::pair_textures() {

	//TODO cleanup with hashmap.
	for (int i = 0; i < materials.size(); i++) {

		for (int j = 0; j < textures.size(); j++) {

			if (textures[j].name == materials[i].albedo_map)
				materials[i].albedoTextureID = j;
			if (textures[j].name == materials[i].emission_map)
				materials[i].emissionTextureID = j;
			if (textures[j].name == materials[i].roughness_map)
				materials[i].roughnessTextureID = j;
			if (textures[j].name == materials[i].metallic_map)
				materials[i].metallicTextureID = j;
			if (textures[j].name == materials[i].opacity_map)
				materials[i].opacityTextureID = j;
			if (textures[j].name == materials[i].normal_map)
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

	try {

	ObjLoader objLoader;

	path += "\\";

	printf("Loading scene \n");

	Scene scene = Scene();

	std::ifstream st(path + "scene.json");

	printf("Opening %s", (path + "scene.json").c_str());

	std::string str((std::istreambuf_iterator<char>(st)),
		std::istreambuf_iterator<char>());


	boost::json::object scene_json = boost::json::parse(str).as_object();

	printf("Loading camera...\n");

	// Camera
	boost::json::object camera_json = scene_json["camera"].as_object();
	boost::json::object camera_pos_json = camera_json["position"].as_object();
	boost::json::object camera_rot_json = camera_json["rotation"].as_object();

	int xRes = camera_json["xRes"].as_int64();
	int yRes = camera_json["yRes"].as_int64();

	float focalLength = camera_json["focalLength"].as_double();
	float focusDistance = camera_json["focusDistance"].as_double();
	float aperture = camera_json["aperture"].as_double();
	bool bokeh = camera_json["bokeh"].as_bool();

	Vector3 cameraPosition = Vector3(camera_pos_json["x"].as_double(), camera_pos_json["y"].as_double(), camera_pos_json["z"].as_double());
	Vector3 cameraRotation = Vector3(camera_rot_json["x"].as_double(), camera_rot_json["y"].as_double(), camera_rot_json["z"].as_double());

	scene.camera = Camera(xRes, yRes);
	scene.camera.focalLength = focalLength;
	scene.camera.focusDistance = focusDistance;
	scene.camera.aperture = aperture;
	scene.camera.position = cameraPosition;
	scene.camera.rotation = cameraRotation;
	scene.camera.bokeh = bokeh;

	printf("Loading HDRI...\n");

	// HDRI

	boost::json::object hdri_json = scene_json["hdri"].as_object();

	if (hdri_json.if_contains("name")) {
		std::cout << "loading hdri from " << hdri_json["name"].as_string() << std::endl;
		scene.addHDRI(hdri_json["name"].as_string().c_str());
	}
	else if (hdri_json.if_contains("color")) {
		Vector3 color = Vector3(hdri_json["color"].as_object()["r"].as_double(), hdri_json["color"].as_object()["g"].as_double(), hdri_json["color"].as_object()["b"].as_double());
		//Vector3 gc_color = Vector3(sycl::pow(color.x, 2.2f), sycl::pow(color.y, 2.2f), sycl::pow(color.z, 2.2f));
		scene.addHDRI(color);
	}


	if (hdri_json.if_contains("xOffset"))
		scene.hdri.texture.xOffset = hdri_json["xOffset"].as_double();

	if (hdri_json.if_contains("yOffset"))
		scene.hdri.texture.xOffset = hdri_json["yOffset"].as_double();

	printf("Loading objs...\n");

	std::vector<UnloadedMaterial> umtls(0);
	std::vector<MeshObject> objects(0);

	objLoader.loadObjsRapid(path + "scene.obj", objects, umtls);
		
	for (int i = 0; i < objects.size(); i++) {
		scene.addMeshObject(objects[i]);	
	}

	printf("%d objects loaded.\n", scene.meshObjectCount());

	scene.pair_materials();

	// PointLights

	/*

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
	*/
	return scene;
	}
	catch (std::exception const& e) {
		BOOST_LOG_TRIVIAL(error) << e.what();
	}


}
