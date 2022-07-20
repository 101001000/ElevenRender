#ifndef POINTLIGHT_H
#define POINTLIGHT_H


// @todo Define useful constructors as temperature, multiply etc
class PointLight{

public:

	Vector3 position;
	Vector3 radiance;

	PointLight(Vector3 _position, Vector3 _radiance) {
		position = _position;
		radiance = _radiance;
	}

};




#endif
