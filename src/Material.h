#pragma once

#include "Vector.h"
#include "Texture.h"

class Material {

public:

	std::string name;

	std::string albedo_map;
	std::string emission_map;
	std::string roughness_map;
	std::string metallic_map;
	std::string normal_map;
	std::string opacity_map;
	std::string transmission_map;

	int albedoTextureID = -1;
	int emissionTextureID = -1;
	int roughnessTextureID = -1;
	int metallicTextureID = -1;
	int normalTextureID = -1;
	int opacityTextureID = -1;
	int transmissionTextureID = -1;

	int albedoShaderID = -1;

	Vector3 albedo = Vector3(0.5,0.5,0.5);
	Vector3 emission = Vector3::Zero();

	float opacity = 1;
	float roughness = 1;
	float metallic = 0;
	float clearcoatGloss = 0;
	float clearcoat = 0;
	float anisotropic = 0;
	float eta = 0;
	float transmission = 0;
	float specular = 0.5;
	float specularTint = 0;
	float sheenTint = 0.5;
	float subsurface = 0;
	float sheen = 0;
	float ax = 0;
	float ay = 0;

	std::string to_string() const;

	inline static Material DefaultMaterial() {
		Material mat;
		mat.name = "default";
		return mat;
	}
};
