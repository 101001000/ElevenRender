#pragma once


#include <string>
#include <CL/sycl.hpp>

class Vector3 {

public:
	float x, y, z;

public:


	inline Vector3(std::string str);

	inline Vector3();

	inline Vector3(float _x, float _y, float _z);

	inline Vector3(float _x);

	inline static Vector3 Zero() { return Vector3(0); }
	inline static Vector3 One() { return Vector3(1); }
	inline static Vector3 Up() { return Vector3(0, 1, 0); }
	inline static Vector3 Down() { return Vector3(0, -1, 0); }
	inline static Vector3 Left() { return Vector3(-1, 0, 0); }
	inline static Vector3 Right() { return Vector3(1, 0, 0); }
	inline static Vector3 Forward() { return Vector3(0, 0, 1); }
	inline static Vector3 Back() { return Vector3(0, 0, -1); }


	SYCL_EXTERNAL inline float length() const;

	SYCL_EXTERNAL inline float operator[](const int n);
	SYCL_EXTERNAL inline Vector3 operator-();


	SYCL_EXTERNAL friend inline Vector3 operator+(const Vector3& v, const float s);
	SYCL_EXTERNAL friend inline Vector3 operator+(const float s, const Vector3& v);
	SYCL_EXTERNAL friend inline Vector3 operator+(const Vector3& v1, const Vector3& v2);

	SYCL_EXTERNAL friend inline Vector3 operator-(const Vector3& v, const float s);
	SYCL_EXTERNAL friend inline Vector3 operator-(const float s, const Vector3& v);
	SYCL_EXTERNAL friend inline Vector3 operator-(const Vector3& v1, const Vector3& v2);

	SYCL_EXTERNAL friend inline Vector3 operator*(const Vector3& v, const float s);
	SYCL_EXTERNAL friend inline Vector3 operator*(const float s, const Vector3& v);
	SYCL_EXTERNAL friend inline Vector3 operator*(const Vector3& v1, const Vector3& v2);

	SYCL_EXTERNAL friend inline Vector3 operator/(const Vector3& v, const float s);
	SYCL_EXTERNAL friend inline Vector3 operator/(const float s, const Vector3& v);
	SYCL_EXTERNAL friend inline Vector3 operator/(const Vector3& v1, const Vector3& v2);

	SYCL_EXTERNAL inline Vector3& operator+=(const Vector3& v);
	SYCL_EXTERNAL inline Vector3& operator-=(const Vector3& v);
	SYCL_EXTERNAL inline Vector3& operator*=(const Vector3& v);
	SYCL_EXTERNAL inline Vector3& operator*=(float s);

	SYCL_EXTERNAL friend inline bool operator==(const Vector3& v1, const Vector3& v2);
	SYCL_EXTERNAL friend inline bool operator!=(const Vector3& v1, const Vector3& v2);
	

	SYCL_EXTERNAL inline static float dot(const Vector3& v1, const Vector3& v2);
	SYCL_EXTERNAL inline static Vector3 cross(const Vector3& v1, const Vector3& v2);
	SYCL_EXTERNAL inline static Vector3 lerp(const Vector3& v1, const Vector3& v2, float amount);
	SYCL_EXTERNAL inline static Vector3 lerp2D(const Vector3& v1, const Vector3& v2, const Vector3& v3, const Vector3& v4, float amountX, float amountY);
	SYCL_EXTERNAL static inline Vector3 reflect(Vector3 v1, Vector3 v2);


	inline void normalize();
	inline Vector3 normalized();

	inline void print();
};


inline Vector3 operator+(const Vector3& v, const float s);
inline Vector3 operator+(const float s, const Vector3& v);
inline Vector3 operator+(const Vector3& v1, const Vector3& v2);

inline Vector3 operator-(const Vector3& v, const float s);
inline Vector3 operator-(const float s, const Vector3& v);
inline Vector3 operator-(const Vector3& v1, const Vector3& v2);

inline Vector3 operator*(const Vector3& v, const float s);
inline Vector3 operator*(const float s, const Vector3& v);
inline Vector3 operator*(const Vector3& v1, const Vector3& v2);

inline Vector3 operator/(const Vector3& v, const float s);
inline Vector3 operator/(const float s, const Vector3& v);
inline Vector3 operator/(const Vector3& v1, const Vector3& v2);

inline bool operator==(const Vector3& v1, const Vector3& v2);
inline bool operator!=(const Vector3& v1, const Vector3& v2);
