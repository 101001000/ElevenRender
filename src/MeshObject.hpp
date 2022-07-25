#ifndef MESHOBJECT_H
#define MESHOBJECT_H

#include "Tri.h"
#include "Ray.hpp"
#include "Hit.h"

static int objectIDCount = 0;

// @todo Define constructor for 


class MeshObject {

public:
	std::string name;
	std::string matName;

	Tri* tris;
	unsigned int triCount;
	int materialID = 0;
	int objectID = 0;

public:

	MeshObject() {
		objectID = objectIDCount++;
	}

	void translate(Vector3 pos) {

		for (int i = 0; i < triCount; i++) {
			tris[i].vertices[0] += pos;
			tris[i].vertices[1] += pos;
			tris[i].vertices[2] += pos;
		}

	}

	inline bool hit(Ray& ray, Hit& hit){

		Hit tempHit = Hit();

		for (int i = 0; i < triCount; i++) {

			if (tris[i].hit(ray, tempHit)) {

				if (!hit.valid) hit = tempHit;

				if((ray.origin - tempHit.position).length() < (ray.origin - hit.position).length()) hit = tempHit;
			}
		}

		if (hit.valid) {
			hit.objectID = objectID;
		}

		return hit.valid;
	}
};

#endif