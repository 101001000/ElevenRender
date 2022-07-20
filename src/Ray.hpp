#ifndef RAY_H
#define RAY_H

#include "Vector.hpp"

class Ray {

public:
	Vector3 origin;
	Vector3 direction;

public:

	inline Ray(Vector3 _origin, Vector3 _direction) {
		origin = _origin;
		direction = _direction;
		direction.normalize();
	}

	inline Ray() {
		origin = Vector3();
		direction = Vector3(0,0,1);
	}

};



#endif