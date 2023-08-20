#include "ObjLoader.h"

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

	std::cout << "ahora a los objs\n";

	if (result.error) {
		std::cout << result.error.code.message() << '\n';
	}

	bool success = rapidobj::Triangulate(result);

	std::map<Vector3, std::vector<Tri>> faces;

	for (const auto& shape : result.shapes) {

		std::cout << "obj " << shape.name << "\n";

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
				tris->push_back(tri);
			}

			mo->name = shape.name;
			mo->tris = tris->data();
			mo->triCount = tris->size();

			if (shape.mesh.material_ids.size() > 0) {

				if (shape.mesh.material_ids[0] >= 0) {

					std::cout << "Linkeando id " << shape.mesh.material_ids[0] << "\n";

					if (result.materials.size() > 0 && shape.mesh.material_ids[0] < result.materials.size())
						mo->matName = result.materials[shape.mesh.material_ids[0]].name;


					std::cout << "matname: " << mo->matName << "\n";
				}
			}

			std::cout << "normales\n";

			if (recompute_normals)
				recompute_normals_face_weight(mo, faces);

			std::cout << "fin normales\n";

			CalcTangents calcTang = CalcTangents();
			calcTang.calc(mo);
			std::cout << "fin calcTang\n";

			meshObjects.push_back(*mo);
		}
	}

	std::cout << "objs hecho\n";
}


void ObjLoader::loadObjsRapid(std::filesystem::path path, std::vector<MeshObject>& meshObjects, bool recompute_normals) {
	loadObjsRapid(rapidobj::ParseFile(path), meshObjects, recompute_normals);
}

void ObjLoader::loadObjsRapid(std::istream& obj_stream, std::string_view material_str, std::vector<MeshObject>& meshObjects, bool recompute_normals) {
	std::cout << material_str;
	loadObjsRapid(rapidobj::ParseStream(obj_stream, rapidobj::MaterialLibrary::String(material_str)), meshObjects, recompute_normals);
}

MeshObject ObjLoader::parseObj(std::ifstream &stream) {

	std::streampos pos = stream.tellg();
	std::string line;
	std::vector<Tri>* tris = new std::vector<Tri>();

	MeshObject mo;

	while (std::getline(stream, line) && line[0] != 'o') {

		if (line.find("usemtl") != std::string::npos) {
			mo.matName = getSecondWord(line);
		}

		if (line[0] == 'v' && line[1] == ' ') {
			vertices.push_back(Vector3(line.substr(2)) * Vector3(1, 1, -1));
		}

		if (line[0] == 'v' && line[1] == 't') {
			textureCoord.push_back(Vector3(line.substr(2)));
		}

		if (line[0] == 'v' && line[1] == 'n') {
			Vector3 n = Vector3(line.substr(2)) * Vector3(1, 1, -1);
			normals.push_back(n);
		}

		if (line[0] == 'f') {

			char f[3][1000];

			f[0][0] = '\0';
			f[1][0] = '\0';
			f[2][0] = '\0';

			Tri tri;

			int idx = -1;

			for (char& c : line) {

				if (isdigit(c) || c == '/') {

					if (idx == -1) { idx = 0; }
					int len = strlen(f[idx]);

					f[idx][len] = c;
					f[idx][len + 1] = '\0';
				}
				else if (idx > -1) {
					idx++;
				}
			}

			for (int i = 0; i < 3; i++) {

				char v[3][100];

				v[0][0] = '\0';
				v[1][0] = '\0';
				v[2][0] = '\0';

				int _idx = 0;

				for (char& c : f[i]) {
					if (c == '\0') { break; }

					if (isdigit(c)) {
						int len = strlen(v[_idx]);

						v[_idx][len] = c;
						v[_idx][len + 1] = '\0';
					}
					else {
						_idx++;
					}
				}


				if (strlen(v[0]) > 0) {
					tri.vertices[i] = vertices.at(std::stoi(&(v[0])[0]) - 1);
				}

				if (strlen(v[1]) > 0) {
					tri.uv[i] = textureCoord.at(std::stoi(&(v[1])[0]) - 1);
				}

				if (strlen(v[2]) > 0) {
					tri.normals[i] = normals.at(std::stoi(&(v[2])[0]) - 1).normalized();
				}
				

			}
			tris->push_back(tri);
		}
	}

	std::cout << "Obj loaded with " << tris->size() << " tris, " << vertices.size() << " vertices and " << normals.size() << " normals\n" << std::endl;

	mo.tris = tris->data();
	mo.triCount = tris->size();

	CalcTangents calcTang = CalcTangents();
	calcTang.calc(&mo);

	for (int i = 0; i < mo.triCount; i++) {

		std::cout << "Vx0: " << mo.tris[i].vertices[0].x << ", y: " << mo.tris[i].vertices[0].y << ", z: " << mo.tris[i].vertices[0].z << std::endl;
		std::cout << "Nx0: " << mo.tris[i].normals[0].x << ", y: " << mo.tris[i].normals[0].y << ", z: " << mo.tris[i].normals[0].z << std::endl;
		std::cout << "Tx0: " << mo.tris[i].tangents[0].x << ", y: " << mo.tris[i].tangents[0].y << ", z: " << mo.tris[i].tangents[0].z << std::endl;
		std::cout << std::endl;
		std::cout << "Vx1: " << mo.tris[i].vertices[1].x << ", y: " << mo.tris[i].vertices[1].y << ", z: " << mo.tris[i].vertices[1].z << std::endl;
		std::cout << "Nx1: " << mo.tris[i].normals[1].x << ", y: " << mo.tris[i].normals[1].y << ", z: " << mo.tris[i].normals[1].z << std::endl;
		std::cout << "Tx1: " << mo.tris[i].tangents[1].x << ", y: " << mo.tris[i].tangents[1].y << ", z: " << mo.tris[i].tangents[1].z << std::endl;
		std::cout << std::endl;
		std::cout << "Vx2: " << mo.tris[i].vertices[2].x << ", y: " << mo.tris[i].vertices[2].y << ", z: " << mo.tris[i].vertices[2].z << std::endl;
		std::cout << "Nx2: " << mo.tris[i].normals[2].x << ", y: " << mo.tris[i].normals[2].y << ", z: " << mo.tris[i].normals[2].z << std::endl;
		std::cout << "Tx2: " << mo.tris[i].tangents[2].x << ", y: " << mo.tris[i].tangents[2].y << ", z: " << mo.tris[i].tangents[2].z << std::endl;
		std::cout << std::endl;
		std::cout << std::endl;

	}

	return mo;
}

std::vector<MeshObject> ObjLoader::loadObjs(std::string path) {

	std::ifstream input(path.c_str());
	std::string line;
	std::vector<MeshObject> meshObjects;

	while (std::getline(input, line)) {

		if (line[0] == 'o') {

			printf("obj");

			std::streampos pos = input.tellg();
			meshObjects.push_back(parseObj(input));
			input.seekg(pos);
		}	
	}
	return meshObjects;
}

std::vector<UnloadedMaterial> ObjLoader::loadMtls(std::string path) {

	std::ifstream input(path.c_str());
	std::string line;
	std::vector<UnloadedMaterial> mtls;

	while (std::getline(input, line)) {
		if (line.find("newmtl") != std::string::npos) {
			std::streampos pos = input.tellg();
			mtls.push_back(parseMtl(input, getSecondWord(line)));
			input.seekg(pos);
		}
	}
	return mtls;
}

