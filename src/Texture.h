#ifndef TEXTURE_H
#define TEXTURE_H

#if !defined(__CUDACC__)
#include <stb_image.h>
#endif

#include "Math.hpp"
#include "sycl.h"

//TODO: refactor, remove loading from filesystem to scene loader.
class Texture {
public:

    enum class Filter { NO_FILTER, BILINEAR };
    enum class CS { LINEAR, sRGB };

    Filter filter = Filter::NO_FILTER;
    // This color need to be removed.
    Vector3 color;

    std::string name;

    int width;
    int height;
    float* data;
    unsigned int channels;

    explicit Texture(std::string filepath);

    Texture(std::string filepath, CS 
        Space);

    std::string to_string();

    void mirror_x();

    void mirror_y();

    // Limit the amount of channels to 3
    void clamp_channels();

    void remove_last_channel();

    // Displaces pixels horizontally or vertically.
    void pixel_shift(const float x_amount, const float y_amount);

    // TODO: This should gamma correct alpha channel?
    void applyGamma(float gamma);

    Texture(int _width, int _height, int _channels, float* _data, Filter _filter);

    explicit Texture(Vector3 _color);

    Texture();

    SYCL_EXTERNAL Vector3 getValueFromCoordinates(int x, int y);

    SYCL_EXTERNAL Vector3 getValueFromUV(float u, float v);

    SYCL_EXTERNAL Vector3 getValueBilinear(float u, float v);

    SYCL_EXTERNAL Vector3 getValueFromUVFiltered(float u, float v);

    SYCL_EXTERNAL static void sphericalMapping(Vector3 origin, Vector3 point, float radius, float& u, float& v);


    SYCL_EXTERNAL Vector3 transformUV(float u, float v);

    SYCL_EXTERNAL Vector3 inverseTransformUV(float u, float v);

    SYCL_EXTERNAL Vector3 reverseSphericalMapping(float u, float v);
};


#endif
