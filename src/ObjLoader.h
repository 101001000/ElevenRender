#pragma once


#include "../third-party/rapidobj.hpp"
#include "../third-party/mikktspaceCallback.h"
#include <vector>
#include <sstream>
#include <iostream>
#include <fstream>

#include "Vector.h"
#include "Material.h"
#include "../third-party/mikktspace.h"
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

	void loadObjsRapid(std::string path, std::vector<MeshObject>& meshObjects, std::vector<UnloadedMaterial>& materials);


	MeshObject parseObj(std::ifstream& stream);

	std::vector<MeshObject> loadObjs(std::string path);

	std::vector<UnloadedMaterial> loadMtls(std::string path);
};
