#include "Vector.h"




inline Vector3::Vector3(std::string str) {
	
		//SE PUEDE SIMPLIFICAR CON UN ÍNDICE

		bool start = false;

		bool px = true;
		bool py = false;
		bool pz = false;

		std::string sx = "";
		std::string sy = "";
		std::string sz = "";

		for (char& c : str) {

			if (isdigit(c) || c == '.' || c == '-') {

				if (px) sx.push_back(c);
				if (py) sy.push_back(c);
				if (pz) sz.push_back(c);

				start = true;
			}
			else {
				if (start) {

					if (px) {
						px = false;
						py = true;
					} else if (py) {
						py = false;
						pz = true;
					} else if (pz) {
						pz = false;
					}
				}
			}
		}

		if (sx.size() == 0) sx.push_back('0');
		if (sy.size() == 0) sy.push_back('0');
		if (sz.size() == 0) sz.push_back('0');

		x = std::stof(sx);
		y = std::stof(sy);
		z = std::stof(sz);
	}

inline Vector3::Vector3() {
		x = 0;
		y = 0;
		z = 0;
	}

inline Vector3::Vector3(float _x, float _y, float _z) {
	x = _x;
	y = _y;
	z = _z;
}

inline Vector3::Vector3(float _x) {
	x = _x;
	y = _x;
	z = _x;
}

inline float Vector3::length() const {
	return sqrt(x * x + y * y + z * z);
}


inline Vector3 operator*(const Vector3& v, const float s) {
	return Vector3(v.x * s, v.y * s, v.z * s);
}
inline Vector3 operator*(const float s, const Vector3& v) {
	return Vector3(v.x * s, v.y * s, v.z * s);
}
inline Vector3 operator*(const Vector3& v1, const Vector3& v2) {
	return Vector3(v1.x * v2.x, v1.y * v2.y, v1.z * v1.z);
}

inline Vector3 operator/(const Vector3& v, const float s) {
	return Vector3(v.x / s, v.y / s, v.z / s);
}
inline Vector3 operator/(const float s, const Vector3& v) {
	return Vector3(v.x / s, v.y / s, v.z / s);
}
inline Vector3 operator/(const Vector3& v1, const Vector3& v2) {
	return Vector3(v1.x / v2.x, v1.y / v2.y, v1.z / v1.z);
}


inline Vector3 operator+(const Vector3& v, const float s) {
	return Vector3(v.x + s, v.y + s, v.z + s);
}
inline Vector3 operator+(const float s, const Vector3& v) {
	return Vector3(v.x + s, v.y + s, v.z + s);
}
inline Vector3 operator+(const Vector3& v1, const Vector3& v2) {
	return Vector3(v1.x + v2.x, v1.y + v2.y, v1.z + v1.z);
}

inline Vector3 operator-(const Vector3& v, const float s) {
	return Vector3(v.x - s, v.y - s, v.z - s);
}
inline Vector3 operator-(const float s, const Vector3& v) {
	return Vector3(v.x - s, v.y - s, v.z - s);
}
inline Vector3 operator-(const Vector3& v1, const Vector3& v2) {
	return Vector3(v1.x - v2.x, v1.y - v2.y, v1.z - v1.z);
}


inline bool operator==(const Vector3& v1, const Vector3& v2) {
	float EPSILON = 0.0001;
	return abs(v1.x - v2.x) < EPSILON && abs(v1.y - v2.y) < EPSILON && abs(v1.z - v2.z) < EPSILON;
}

inline bool operator!=(const Vector3& v1, const Vector3& v2) {
	return !(v1 == v2);
}

inline Vector3 Vector3::operator-() {
	return *this * -1;
}


inline float Vector3::operator[](const int n) {
	if (n == 0) return x;
	if (n == 1) return y;
	if (n == 2) return z;
	return x;
}

inline Vector3& Vector3::operator+=(const Vector3& v) {
	x += v.x;
	y += v.y;
	z += v.z;
	return *this;
}

inline Vector3& Vector3::operator-=(const Vector3& v) {
	x -= v.x;
	y -= v.y;
	z -= v.z;
	return *this;
}

inline Vector3& Vector3::operator*=(const Vector3& v) {
	x *= v.x;
	y *= v.y;
	z *= v.z;
	return *this;
}

inline Vector3& Vector3::operator*=(float s) {
	x *= s;
	y *= s;
	z *= s;
	return *this;
}

inline float Vector3::dot(const Vector3& v1, const Vector3& v2) {
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

inline Vector3 Vector3::cross(const Vector3& v1, const Vector3& v2) {
	return Vector3((v1.y * v2.z - v1.z * v2.y), - (v1.x * v2.z - v1.z * v2.x), (v1.x * v2.y - v1.y * v2.x));
}

inline void Vector3::normalize() {

	float l = length();

	if (l == 0) return;

	x /= l;	y /= l;	z /= l;
}

inline Vector3 Vector3::normalized() {
	if (length() == 0) return Vector3(x, y, z);
	return Vector3(x, y, z) / length();
}


inline void Vector3::print() {
	printf("x: %.3f, y: %.3f, z: %.3f.", x, y, z);
}

inline Vector3 reflect(Vector3 v1, Vector3 v2) {
	return v1 - v2 * 2 * (Vector3::dot(v1, v2));
}

inline Vector3 Vector3::lerp(const Vector3& v1, const Vector3& v2, float amount) {
	return amount * v1 + (1 - amount) * v2;
}

inline Vector3 Vector3::lerp2D(const Vector3& v1, const Vector3& v2, const Vector3& v3, const Vector3& v4, float amountX, float amountY) {
	return lerp(lerp(v1, v2, amountX), lerp(v3, v4, amountX), amountY);
}
