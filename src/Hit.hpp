#ifndef HIT_H
#define HIT_H

#include "Vector.h"

class Hit {

public:
	Vector3 position, normal, tangent, bitangent;
	bool valid = false;
	unsigned int objectID;
	float tu, tv;
};

#endif