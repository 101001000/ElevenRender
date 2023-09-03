#pragma once

#include "Vector.h"
#include "Math.hpp"
#include "sycl.h"

//TODO naming convention

inline Vector3 uniformSampleSphere(float u1, float u2) {

    float z = 1.0f - 2.0f * u1;
    float r = sycl::sqrt(maxf(0.f, 1.0f - z * z));
    float phi = 2.0f * PIF * u2;
    float x = r * sycl::cos(phi);
    float y = r * sycl::sin(phi);

    return Vector3(x, y, z);
}

inline void uniformCircleSampling(float u1, float u2, float u3, float& x, float& y) {

    float t = 2 * PIF * u1;
    float u = u2 + u3;
    float r = u > 1 ? 2 - u : u;
       
    x = r * sycl::cos(t);
    y = r * sycl::sin(t);
}

inline Vector3 CosineSampleHemisphere(float u1, float u2){

    Vector3 dir;
    float r = sycl::sqrt(u1);
    float phi = 2.0f * PIF * u2;
    dir.x = r * sycl::cos(phi);
    dir.y = r * sycl::sin(phi);
    dir.z = sycl::sqrt(maxf(0.0f, 1.0f - dir.x * dir.x - dir.y * dir.y));

    return dir;
}

inline Vector3 ImportanceSampleGGX(float rgh, float r1, float r2) {
    float a = maxf(0.001f, rgh);

    float phi = r1 * PIF * 2;

    float cosTheta = sycl::sqrt((1.0f - r2) / (1.0f + (a * a - 1.0f) * r2));
    float sinTheta = clamp(sycl::sqrt(1.0f - (cosTheta * cosTheta)), 0.0f, 1.0f);
    float sinPhi = sycl::sin(phi);
    float cosPhi = sycl::cos(phi);

    return Vector3(sinTheta * cosPhi, sinTheta * sinPhi, cosTheta);
}
