#include "Camera.h"



Camera::Camera(unsigned int _xRes, unsigned int _yRes) {
	xRes = _xRes;
	yRes = _yRes;
	setSensorWidth(sensorWidth);
}

void Camera::setSensorWidth(float size) {
	sensorHeight = size * (static_cast<float>(yRes) / static_cast<float>(xRes));
}
