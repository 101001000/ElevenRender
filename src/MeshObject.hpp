#ifndef MESHOBJECT_H
#define MESHOBJECT_H

#include "Tri.h"
#include "Ray.h"
#include "Hit.h"
#include <iostream>

static int objectIDCount = 0;

// @todo Define constructor for 


class MeshObject {

public:
	std::string name;

	Tri* tris;
	unsigned int triCount;
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

	// Cheap solution for normal weighting. Currently just size weight: http://www.bytehazard.com/articles/vertnorm.html

	void recomputeNormals() {

		for (int i = 0; i < triCount; i++) {

			for (int j = 0; j < 3; j++) {

				Vector3 v = tris[i].vertices[j];
				Vector3 n;

				for (int ii = 0; ii < triCount; ii++) {
					for (int jj = 0; jj < 3; jj++) {
						if (v == tris[ii].vertices[jj]) {
							Vector3 edge2 = tris[ii].vertices[2] - tris[ii].vertices[0];
							Vector3 edge1 = tris[ii].vertices[1] - tris[ii].vertices[0];
							n += Vector3::cross(edge2, edge1);
							break;
						}
					}
				}
				
				//std::cout << "old normal: " << tris[i].normals[j].x << " " << tris[i].normals[j].y << " " << tris[i].normals[j].z << " \n";
				tris[i].normals[j] = n.normalized();
				//std::cout << "new normal: " << tris[i].normals[j].x << " " << tris[i].normals[j].y << " " << tris[i].normals[j].z << " \n";
				//std::cout << "\n";
			}
		}

	}
};

#endif