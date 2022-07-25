#ifndef  OBJLOADER_H
#define OBJLOADER_H

#include "libs/rapidobj.hpp"
#include "libs/mikktspaceCallback.h"
#include <vector>
#include <sstream>
#include <iostream>
#include <fstream>

#include "Vector.h"
#include "libs/mikktspace.h"
#include "Tri.hpp"
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

	static std::string getSecondWord(std::string str) {
		std::string::size_type in = str.find_first_of(" ");
		return str.substr(in + 1, str.length());
	}

	UnloadedMaterial parseMtl(std::ifstream& stream, std::string name) {

		UnloadedMaterial umtl;

		umtl.mat.name = name;

		std::string line;
		while (std::getline(stream, line) && !(line.find("newmtl") != std::string::npos)) {

			//Ns TODO this is an exponent for specular
			//Ka TODO --> Ambient color
			//Kd
			if (line[0] == 'K' && line[1] == 'd')
				umtl.mat.albedo = Vector3(line.substr(2));
			//Ks TODO find a way to make this compatible
			if (line[0] == 'K' && line[1] == 's')
				umtl.mat.specular = Vector3(line.substr(2)).x;
			//Ke
			if (line[0] == 'K' && line[1] == 'e')
				umtl.mat.emission = Vector3(line.substr(2));
			//Ni
			if (line[0] == 'N' && line[1] == 'i')
				umtl.mat.eta = std::stof(getSecondWord(line));
			//d
			if (line[0] == 'd')
				umtl.mat.opacity = std::stof(getSecondWord(line)); // Implicit type conversion, check if it's appropiate
			
			std::vector<std::string> mapNames = { "map_Kd", "map_Ns", "map_Bump", "refl" };

			for (int i = 0; i < mapNames.size(); i++) {
				if (line.find(mapNames[i]) != std::string::npos)
					umtl.maps[mapNames[i]] = getSecondWord(line);
			}
		}

		return umtl;
	}

	void loadObjsRapid(std::string path, std::vector<MeshObject>& meshObjects, std::vector<UnloadedMaterial>& materials) {

		auto result = rapidobj::ParseFile(path.c_str());

        if (result.error) {
            std::cout << result.error.code.message() << '\n';
        }

        bool success = rapidobj::Triangulate(result);

		for (const auto& mat : result.materials) {

			std::cout << "loading mat " << mat.name << " with albedo " << mat.diffuse[0] << " - " << mat.diffuse[1] << " - " << mat.diffuse[2] << std::endl;

			UnloadedMaterial umtl;
			umtl.mat.name = mat.name;
			umtl.mat.albedo = Vector3(mat.diffuse[0], mat.diffuse[1], mat.diffuse[2]);

			materials.push_back(umtl);
		}

        for (const auto& shape : result.shapes) {

			std::vector<Tri>* tris = new std::vector<Tri>();
			MeshObject mo;

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

				/*

				std::cout << "Pushing back v " << tri.vertices[0].x << ", " << tri.vertices[0].y << ", " << tri.vertices[0].z << ", " << tri.vertices[1].x << ", " << tri.vertices[1].y << ", " << tri.vertices[2].z << ", " << tri.vertices[2].x << ", " << tri.vertices[2].y << ", " << tri.vertices[2].z << std::endl;
				std::cout << "Pushing back n " << tri.normals[0].x << ", " << tri.normals[0].y << ", " << tri.normals[0].z << ", " << tri.normals[1].x << ", " << tri.normals[1].y << ", " << tri.normals[2].z << ", " << tri.normals[2].x << ", " << tri.normals[2].y << ", " << tri.normals[2].z << std::endl;
				std::cout << "Pushing back t " << tri.uv[0].x << ", " << tri.uv[0].y << ", " << tri.uv[1].x << ", " << tri.uv[1].y << ", " << tri.uv[2].x << ", " << tri.uv[2].y << std::endl;

				*/

				tris->push_back(tri);
			}

			mo.name = shape.name;
			mo.tris = tris->data();
			mo.triCount = tris->size();
	
			if (shape.mesh.material_ids[0] >= 0)
				mo.matName = result.materials[shape.mesh.material_ids[0]].name;

			CalcTangents calcTang = CalcTangents();
			calcTang.calc(&mo);

			meshObjects.push_back(mo);
        }
    }
	

	MeshObject parseObj(std::ifstream &stream) {

		std::streampos pos = stream.tellg();
		std::string line;
		std::vector<Tri>* tris = new std::vector<Tri>();

		MeshObject mo;

		while (std::getline(stream, line) && line[0] != 'o') {

			if (line.find("usemtl") != std::string::npos)
				mo.matName = getSecondWord(line);

			if (line[0] == 'v' && line[1] == ' ')
				vertices.push_back(Vector3(line.substr(2)) * Vector3(1, 1, -1));

			if (line[0] == 'v' && line[1] == 't')
				textureCoord.push_back(Vector3(line.substr(2)));

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

						if (idx == -1) idx = 0;
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
						if (c == '\0') break;

						if (isdigit(c)) {
							int len = strlen(v[_idx]);

							v[_idx][len] = c;
							v[_idx][len + 1] = '\0';
						}
						else {
							_idx++;
						}
					}


					if (strlen(v[0]) > 0)
						tri.vertices[i] = vertices.at(std::stoi(&(v[0])[0]) - 1);

					if (strlen(v[1]) > 0)
						tri.uv[i] = textureCoord.at(std::stoi(&(v[1])[0]) - 1);

					if (strlen(v[2]) > 0) 
						tri.normals[i] = normals.at(std::stoi(&(v[2])[0]) - 1).normalized();
					

				}
				tris->push_back(tri);
			}
		}

		std::cout << "Obj loaded with " << tris->size() << " tris, " << vertices.size() << " vertices and " << normals.size() << " normals\n" << std::endl;

		mo.tris = tris->data();
		mo.triCount = tris->size();

		CalcTangents calcTang = CalcTangents();
		calcTang.calc(&mo);
	
		return mo;
	}

	std::vector<MeshObject> loadObjs(std::string path) {

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

	std::vector<UnloadedMaterial> loadMtls(std::string path) {

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
};





#endif
