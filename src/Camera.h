#pragma once

#include "Vector.h"

class Camera {

public:

	float focalLength = 35 * 0.001;
	float sensorWidth = 36 * 0.001;
	float sensorHeight = 24 * 0.001;
	float aperture = 2.8;
	float focusDistance = 1000000;

	Vector3 rotation;

	bool bokeh = false;

	Vector3 position = Vector3::Zero();

public:

	Camera() {}

};

