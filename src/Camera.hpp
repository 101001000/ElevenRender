#ifndef CAMERA_H
#define CAMERA_H

#include "Vector.hpp"

class Camera {

public:

	unsigned int xRes = 1280;
	unsigned int yRes = 720;

	float focalLength = 35 * 0.001;
	float sensorWidth = 35 * 0.001;
	float sensorHeight = sensorWidth * ((float)yRes / (float)xRes);
	float aperture = 2.8;
	float focusDistance = 1000000;

	Vector3 rotation;

	bool bokeh = false;

	Vector3 position = Vector3::Zero();

public:

	Camera() {}

	Camera(unsigned int _xRes, unsigned int _yRes) {
		xRes = _xRes;
		yRes = _yRes;
		setSensorWidth(sensorWidth);
	}

	void setSensorWidth(float size) {
		sensorHeight = size * ((float)yRes / (float)xRes);
	}


};


#endif
