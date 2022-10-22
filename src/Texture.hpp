#ifndef TEXTURE_H
#define TEXTURE_H

#if !defined(__CUDACC__)
#include <stb_image.h>
#endif


#include "Math.hpp"
#include <sycl.h>

//TODO: refactor, remove loading from filesystem to scene loader.
class Texture {
public:

    enum class Filter { NO_FILTER, BILINEAR };
    enum class CS { LINEAR, sRGB };

	float* data;

    Filter filter = Filter::NO_FILTER;
    Vector3 color;

    std::string name;

    int width;
    int height;

    float xTile = 1;
    float yTile = 1;

    float xOffset = 0;
    float yOffset = 0;

    explicit Texture(std::string filepath) : Texture(filepath, CS::sRGB) {}

    //TODO fix this compilation nightmare
#if !defined(__CUDACC__)
    Texture(std::string filepath, CS colorSpace) {

        if (colorSpace == CS::sRGB) {
            stbi_ldr_to_hdr_gamma(2.2f);
        }

        else if (colorSpace == CS::LINEAR) {
            stbi_ldr_to_hdr_gamma(1.0f);
        }
 
        stbi_set_flip_vertically_on_load(true);

        printf("Loading texture from %s... ", filepath.c_str());

        name = filepath;

        int channels;
        float* tmp_data = stbi_loadf(filepath.c_str(), &width, &height, &channels, 0);

        data = new float[width * height * 3];

        for (int i = 0, j = 0; i < width * height * 3; i += 3, j += channels) {
            data[i + 0] = (tmp_data[j + 0]);
            data[i + 1] = (tmp_data[j + 1]);
            data[i + 2] = (tmp_data[j + 2]);
        }

        printf("Loaded! %dpx x %dpx, %d channels\n", width, height, channels);
        stbi_image_free(tmp_data);
    }
#else
    Texture(std::string filepath, CS colorSpace) {
        printf("COMPILATION MISMATCH");
    }
       
#endif


    void applyGamma(float gamma) {
        for (int i = 0; i < width * height * 3; i++) {
            data[i] = fast_pow(data[i], gamma);
        }
    }

    // Quick fix for passing blender data, not ideal
    Texture(int _width, int _height, int _channels, float* _data) {
        width = _width;
        height = _height;
        data = new float[width * height * 3];

        if (_channels == 4) {
            //remove alpha channel
            for (int i = 0; i < width * height; i++) {
                data[i * 3 + 0] = _data[i * 4 + 0];
                data[i * 3 + 1] = _data[i * 4 + 1];
                data[i * 3 + 2] = _data[i * 4 + 2];
            }
        }
        else if (_channels == 3) {
            memcpy(data, _data, sizeof(float) * width * height * 3);
        }
    }

    explicit Texture(Vector3 _color) {

        width = 1; height = 1;

        color = _color;
        data = new float[3];
        data[0] = color.x; data[1] = color.y; data[2] = color.z;
    }

    Texture() {
        
        width = 1;
        height = 1;

        data = new float[3];

        data[0] = 0; data[1] = 0; data[2] = 0;
    }

    Vector3 getValueFromCoordinates(int x, int y) {

        Vector3 pixel;

        // Offset and tiling tranformations
        x = static_cast<int>(xTile * (x + xOffset * width)) % width;
        y = static_cast<int>(yTile * (y + yOffset * height)) % height;

        if (x < 0) {
            x = 0;
        }
        if (y < 0) {
            y = 0;
        }

        pixel.x = data[(3 * (y * width + x) + 0)];
        pixel.y = data[(3 * (y * width + x) + 1)];
        pixel.z = data[(3 * (y * width + x) + 2)];

        return pixel;
    }

    Vector3 getValueFromUV(float u, float v) {
        return getValueFromCoordinates(u * width, v * height);
    }

    Vector3 getValueBilinear(float u, float v) {
        
        float x = u * width;
        float y = v * height;

        float t1x = floor(x);
        float t1y = floor(y);

        float t2x = t1x + 1;
        float t2y = t1y + 1;

        float a = (x - t1x) / (t2x - t1x);
        float b = (y - t1y) / (t2y - t1y);

        Vector3 v1 = getValueFromCoordinates(t1x, t1y);
        Vector3 v2 = getValueFromCoordinates(t2x, t1y);
        Vector3 v3 = getValueFromCoordinates(t1x, t2y);
        Vector3 v4 = getValueFromCoordinates(t2x, t2y);

        // Linear interpolation
        return lerp(lerp(v1, v2, a), lerp(v3, v4, a), b);
	}

    Vector3 getValueFromUVFiltered(float u, float v) {
        if (filter == Filter::BILINEAR) {
            return getValueBilinear(u, v);
        }
        else {
            return getValueFromUV(u, v);
        }
    }

    static inline void sphericalMapping(Vector3 origin, Vector3 point, float radius, float& u, float& v) {

        // Point is normalized to radius 1 sphere
        Vector3 p = (point - origin) / radius;

        float theta = sycl::acos(-p.y);
        float phi = sycl::atan2(-p.z, p.x) + PI;

        u = phi / (2 * PI);
        v = theta / PI;

        limitUV(u,v);
    }

    inline Vector3 transformUV(float u, float v) {

        int x = u * width;
        int y = v * height;

        // OJO TILE

        x = static_cast<int>(xTile * (x + xOffset * width)) % width;
        y = static_cast<int>(yTile * (y + yOffset * height)) % height;


        float nu = static_cast<float>(x) / static_cast<float>(width);
        float nv = static_cast<float>(y) / static_cast<float>(height);

        limitUV(nu, nv);

        return Vector3(nu, nv, 0);
    }

    inline Vector3 inverseTransformUV(float u, float v) {

        int x = u * width;
        int y = v * height;

        // OJO TILE

        x = static_cast<int>(xTile * (x - xOffset * width)) % width;
        y = static_cast<int>(yTile * (y - yOffset * height)) % height;

        float nu = static_cast<float>(x) / static_cast<float>(width);
        float nv = static_cast<float>(y) / static_cast<float>(height);

        limitUV(nu, nv);

        return Vector3(nu, nv, 0);
    }

    inline Vector3 reverseSphericalMapping(float u, float v) {

        float phi = u * 2 * PI;
        float theta = v * PI;

        float px = sycl::cos(phi - PI);
        float py = -sycl::cos(theta);
        float pz = -sycl::sin(phi - PI);

        float a = sqrt(1 - py * py);

        return Vector3(a * px, py, a * pz);
    }
};


#endif