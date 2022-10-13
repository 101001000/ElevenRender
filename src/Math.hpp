#ifndef MATH_H
#define MATH_H

#include "Vector.h"

#define PI 3.14159265358979323846f
#define TWO_PI 6.28318530717958647692f

#define FAST_LERP

// https://martin.ankerl.com/2012/01/25/optimized-approximative-pow-in-c-and-cpp/
inline double fast_pow(double a, double b) {
    union {
        double d;
        int x[2];
    } u = { a };
    u.x[1] = (int)(b * (u.x[1] - 1072632447) + 1072632447);
    u.x[0] = 0;
    return u.d;
}

static float map(float a, float b, float c, float d, float e) {
    return d + ((a - b) / (c - b)) * (e - d);
}

static Vector3 map(Vector3 a, Vector3 b, Vector3 c, Vector3 d, Vector3 e) {
    return Vector3(map(a.x, b.x, c.x, d.x, e.x), map(a.y, b.y, c.y, d.y, e.y), map(a.z, b.z, c.z, d.z, e.z));
}

static float clamp(float a, float b, float c) {
    return a < b ? b : a > c ? c : a;
}

static Vector3 clamp(Vector3 v, float b, float c) {
    return Vector3(clamp(v.x, b, c), clamp(v.y, b, c), clamp(v.z, b, c));
}

static float lerp(float a, float b, float c) {
#ifdef FAST_LERP
    // Este método no se comporta bien en algunos casos de infinitos y nans
    return a + c * (b - a);
#else
    return b * c + a * (1 - c);
#endif
}


inline static void limitUV(float& u, float& v) {
    u += -(u > 1) + -(u < 0);
    v += -(v > 1) + -(v < 0);
}

static Vector3 sqrt(Vector3 v) {
    return Vector3(sqrt(v.x), sqrt(v.y), sqrt(v.z));
}

static Vector3 lerp(Vector3 a, Vector3 b, float c) {
    return Vector3(lerp(a.x, b.x, c), lerp(a.y, b.y, c), lerp(a.z, b.z, c));
}

static float minf(float a, float b) {
    return a < b ? a : b;
}

static float maxf(float a, float b) {
    return a > b ? a : b;
}



#endif