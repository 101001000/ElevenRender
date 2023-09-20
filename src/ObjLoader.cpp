#include "ObjLoader.h"
#include "Logging.h"
#include <regex>

std::string ObjLoader::getSecondWord(std::string str) {
	std::string::size_type in = str.find_first_of(" ");
	return str.substr(in + 1, str.length());
}

UnloadedMaterial ObjLoader::parseMtl(std::ifstream& stream, std::string name) {

	UnloadedMaterial umtl;

	umtl.mat.name = name;

	std::string line;
	while (std::getline(stream, line) && !(line.find("newmtl") != std::string::npos)) {

		//Ns TODO this is an exponent for specular
		//Ka TODO --> Ambient color
		//Kd
		if (line[0] == 'K' && line[1] == 'd') {
			umtl.mat.albedo = Vector3(line.substr(2));
		}
		//Ks TODO find a way to make this compatible
		if (line[0] == 'K' && line[1] == 's') {
			umtl.mat.specular = Vector3(line.substr(2)).x;
		}
		//Ke
		if (line[0] == 'K' && line[1] == 'e') {
			umtl.mat.emission = Vector3(line.substr(2));
		}
		//Ni
		if (line[0] == 'N' && line[1] == 'i') {
			umtl.mat.eta = std::stof(getSecondWord(line));
		}
		//d
		if (line[0] == 'd') {
			umtl.mat.opacity = std::stof(getSecondWord(line)); // Implicit type conversion, check if it's appropiate
		}
		std::vector<std::string> mapNames = { "map_Kd", "map_Ns", "map_Bump", "refl" };

		for (int i = 0; i < mapNames.size(); i++) {
			if (line.find(mapNames[i]) != std::string::npos) {
				umtl.maps[mapNames[i]] = getSecondWord(line);
			}
		}
	}

	return umtl;
}

void recompute_normals_face_weight(MeshObject* mo, std::map<Vector3, std::vector<Tri>>& faces) {
	for (int i = 0; i < mo->triCount; i++) {
		for (int j = 0; j < 3; j++) {
			Vector3 v = mo->tris[i].vertices[j];
			Vector3 n;
			for (int f = 0; f < faces[v].size(); f++) {
				Vector3 edge2 = faces[v][f].vertices[2] - faces[v][f].vertices[0];
				Vector3 edge1 = faces[v][f].vertices[1] - faces[v][f].vertices[0];
				n += Vector3::cross(edge2, edge1);
			}
			mo->tris[i].normals[j] = n.normalized();
		}
	}
}


void ObjLoader::loadObjsRapid(rapidobj::Result result, std::vector<MeshObject>& meshObjects, bool recompute_normals) {

	if (result.error) {
		LOG(error) << result.error.code.message() << " in " << result.error.line << " : " << result.error.line_num;
	}

	

	bool success = rapidobj::Triangulate(result);

	std::map<Vector3, std::vector<Tri>> faces;

	for (const auto& shape : result.shapes) {

		if (shape.lines.indices.size() == 0) {

			std::vector<Tri>* tris = new std::vector<Tri>();
			MeshObject* mo = new MeshObject();

			for (int i = 0; i < shape.mesh.indices.size(); i += 3) {

				Tri tri;

				for (int j = 0; j < 3; j++) {

					rapidobj::Index index = shape.mesh.indices[i + j];

					int p_idx = index.position_index;
					int n_idx = index.normal_index;
					int t_idx = index.texcoord_index;

					float px = result.attributes.positions[3 * p_idx + 0];
					float py = result.attributes.positions[3 * p_idx + 1];
					float pz = result.attributes.positions[3 * p_idx + 2];

					float nx = result.attributes.normals[3 * n_idx + 0];
					float ny = result.attributes.normals[3 * n_idx + 1];
					float nz = result.attributes.normals[3 * n_idx + 2];

					float tu = result.attributes.texcoords[2 * t_idx + 0];
					float tv = result.attributes.texcoords[2 * t_idx + 1];

					tri.vertices[j] = Vector3(px, py, -pz);
					tri.normals[j] = Vector3(nx, ny, -nz).normalized();
					tri.uv[j] = Vector3(tu, tv, 0);
				}

				if (recompute_normals) {
					faces[tri.vertices[0]].push_back(tri);
					faces[tri.vertices[1]].push_back(tri);
					faces[tri.vertices[2]].push_back(tri);
				}

				int face_id = i / 3;
				int material_id = -1;

				if (face_id < shape.mesh.material_ids.size())
					material_id = shape.mesh.material_ids[face_id];

				if(material_id < result.materials.size() && material_id > -1)
					tri.matName = result.materials[material_id].name;

				tris->push_back(tri);
			}

			mo->name = shape.name;
			mo->tris = tris->data();
			mo->triCount = tris->size();

			if (recompute_normals)
				recompute_normals_face_weight(mo, faces);

			CalcTangents calcTang = CalcTangents();
			calcTang.calc(mo);

			meshObjects.push_back(*mo);
		}
	}
}


void ObjLoader::loadObjsRapid(std::filesystem::path path, std::vector<MeshObject>& meshObjects, bool recompute_normals) {
	loadObjsRapid(rapidobj::ParseFile(path), meshObjects, recompute_normals);
}

void ObjLoader::loadObjsRapid(std::istream& obj_stream, std::string_view material_str, std::vector<MeshObject>& meshObjects, bool recompute_normals) {
	std::string str = std::string(material_str);
	std::regex pattern("^((?!newmtl).)*$", std::regex::multiline);
	std::string result = std::regex_replace(str, pattern, "");

	//std::string result = std::regex_replace(str, std::regex("\\map_Bump"), "map_bump");
	//std::string result2 = std::regex_replace(result, std::regex("\\map_refl"), "refl");
	//std::string result3 = std::regex_replace(result2, std::regex("\\map_refl"), "refl");
	//std::string result4 = std::regex_replace(result3, std::regex("\\map_Kd"), "");
	std::cout << result;
	loadObjsRapid(rapidobj::ParseStream(obj_stream, rapidobj::MaterialLibrary::String(result)), meshObjects, recompute_normals);
}
