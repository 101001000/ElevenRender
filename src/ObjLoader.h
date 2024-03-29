#pragma once


#include <rapidobj/rapidobj.hpp>
#include "mikktspaceCallback.h"
#include <vector>
#include <sstream>
#include <iostream>
#include <fstream>
#include <filesystem>

#include "Vector.h"
#include "Material.h"
#include <mikktspace.h>
#include "Tri.h"
#include "MeshObject.hpp"

struct UnloadedMaterial {
	Material mat;
	std::map<std::string, std::string> maps;
};

class ObjLoader {

public:

	std::vector<Vector3> vertices;
	std::vector<Vector3> textureCoord;
	std::vector<Vector3> normals;

	static std::string getSecondWord(std::string str);

	UnloadedMaterial parseMtl(std::ifstream& stream, std::string name);

	void loadObjsRapid(std::filesystem::path path, std::vector<MeshObject>& meshObjects, bool recompute_normals);
	void loadObjsRapid(std::istream& obj_stream, std::string_view material_str, std::vector<MeshObject>& meshObjects, bool recompute_normals);
	void loadObjsRapid(rapidobj::Result result, std::vector<MeshObject>& meshObjects, bool recompute_normals);


	MeshObject parseObj(std::ifstream& stream);

	std::vector<MeshObject> loadObjs(std::string path);

	std::vector<UnloadedMaterial> loadMtls(std::string path);
};
