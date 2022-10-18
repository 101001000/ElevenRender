#pragma once
#include "Vector.h"

class Hit {

public:
	Vector3 position, normal, tangent, bitangent, gnormal;
	bool valid = false;
	unsigned int objectID;
	float tu, tv;
	int triIdx = -2;
};
