#include "Texture.h"



    Texture::Texture(std::string filepath) : Texture(filepath, CS::sRGB) {}

    //TODO fix this compilation nightmare
#if !defined(__CUDACC__)
    Texture::Texture(std::string filepath, CS colorSpace) {

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
    Texture::Texture(std::string filepath, CS colorSpace) {
        printf("COMPILATION MISMATCH");
    }
       
#endif

    std::string Texture::to_string() {
        //TODO: Add filtering and color_space.

       std::string str = "Texture " + name + " (" + std::to_string(width) + "x" + std::to_string(height) + "), " + std::to_string(channels) + " channels";

       //for (int i = 0; i < width * height * channels; i++) {
       //    str += std::to_string(data[i]);
       //    str += ",";
       //}

       return str;
    }


    void Texture::mirror_x() {
        float* new_data = new float[width * height * channels];
        for (int x = 0; x < width; x++) {
            for (int y = 0; y < height; y++) {
                for (int c = 0; c < channels; c++) {
                    new_data[channels*(y * width + x) + c] = data[channels*(y * width + (width-x-1)) + c];
                }
            }
        }
        delete[] data;
        data = new_data;
    }

    void Texture::mirror_y() {
        float* new_data = new float[width * height * channels];
        for (int x = 0; x < width; x++) {
            for (int y = 0; y < height; y++) {
                for (int c = 0; c < channels; c++) {
                    new_data[channels * (y * width + x) + c] = data[channels * ((height-y-1) * width + x) + c];
                }
            }
        }
        delete[] data;
        data = new_data;
    }

    // Limit the amount of channels to 3
    void Texture::clamp_channels() {
        std::cout << "Clamping channels" << std::endl;
        // Channel count is modified when removing one channel.
        int or_channels = channels;
        for (int i = 3; i < or_channels; i++) {
            std::cout << "Removing one channel" << std::endl;
            remove_last_channel();
            std::cout << "Removed!" << std::endl;
        }
    }

    void Texture::remove_last_channel() {
        float* new_pixels = new float[width * height * (channels - 1)];
        for (int x = 0; x < width; x++) {
            for (int y = 0; y < height; y++) {
                for (int c = 0; c < channels - 1; c++) {
                    new_pixels[(channels - 1) * (y * width + x) + c] = data[channels * (y * width + x) + c];
                }
            }
        }
        channels--;
        delete[] data;
        data = new_pixels;
    }

    // Displaces pixels horizontally or vertically.
    void Texture::pixel_shift(const float x_amount, const float y_amount) {
        float* shifted_pixels = new float[width * height * channels];
        for (int x = 0; x < width; x++) {
            int shifted_x = static_cast<int>(x + width * x_amount) % width;
            for (int y = 0; y < height; y++) {
                int shifted_y = static_cast<int>(y + height * y_amount) % height;
                for (int c = 0; c < channels; c++) {
                    shifted_pixels[channels * (shifted_y * width + shifted_x) + c] = data[channels * (y * width + x) + c];
                }
            }
        }
        delete[] data;
        data = shifted_pixels;
    }

    // TODO: This should gamma correct alpha channel?
    void Texture::applyGamma(float gamma) {
        for (int i = 0; i < width * height * channels; i++) {
            data[i] = fast_pow(data[i], gamma);
        }
    }



    static float sRGBToLinear(float s) {
        if (s <= 0.04045f) {
            return s / 12.92f;
        }
        else {
            return fast_pow((s + 0.055f) / 1.055f, 2.4f);
        }
    }

    Texture::Texture(std::string _name, int _width, int _height, int _channels, float* _data, Filter _filter, CS _cs) :
        name(_name), width(_width), height(_height), channels(_channels), filter(_filter), data(_data) {
        if (_cs == CS::sRGB) {
            for (int i = 0; i < width * height * channels; i++)
                data[i] = sRGBToLinear(data[i]);
        }

    }

    Texture::Texture(Vector3 _color) {

        width = 1; height = 1;
        channels = 3;
        color = _color;
        data = new float[3];
        data[0] = color.x; data[1] = color.y; data[2] = color.z;
    }

    Texture::Texture() {
        width = 1;
        height = 1;
        channels = 1;
        data = new float[1];
        data[0] = 0;
    }

    Vector3 Texture::getValueFromCoordinates(int x, int y) {

        Vector3 pixel;

        x %= width;
        y %= height;

        if (x < 0) x *= -1;
        if (y < 0) y *= -1;

        if (channels == 0) {
            pixel = Vector3();
        }
        else if (channels == 1) {
            pixel = Vector3(data[y * width + x]);
        }
        else if (channels == 2) {
            pixel.x = data[(channels * (y * width + x) + 0)];
            pixel.y = data[(channels * (y * width + x) + 1)];
        }
        // Ignore >3 channels
        else {
            pixel.x = data[(channels * (y * width + x) + 0)];
            pixel.y = data[(channels * (y * width + x) + 1)];
            pixel.z = data[(channels * (y * width + x) + 2)];
        }

        return pixel;
    }

    Vector3 Texture::getValueFromUV(float u, float v) {
        return getValueFromCoordinates(u * width, v * height);
    }

    Vector3 Texture::getValueBilinear(float u, float v) {
        
        float x = u * width;
        float y = v * height;

        float t1x = sycl::floor(x);
        float t1y = sycl::floor(y);

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

    Vector3 Texture::getValueFromUVFiltered(float u, float v) {
        if (filter == Filter::BILINEAR) {
            return getValueBilinear(u, v);
        }
        else {
            return getValueFromUV(u, v);
        }
    }


    void Texture::sphericalMapping(Vector3 origin, Vector3 point, float radius, float& u, float& v) {

        // Point is normalized to radius 1 sphere
        Vector3 p = (point - origin) / radius;

        float theta = sycl::acos(-p.y);
        float phi = sycl::atan2(-p.z, p.x) + PIF;

        u = phi / (2 * PIF);
        v = theta / PIF;

        limitUV(u,v);
    }


    Vector3 Texture::transformUV(float u, float v) {

        int x = u * width;
        int y = v * height;

        float nu = static_cast<float>(x) / static_cast<float>(width);
        float nv = static_cast<float>(y) / static_cast<float>(height);

        limitUV(nu, nv);

        return Vector3(nu, nv, 0);
    }

    Vector3 Texture::inverseTransformUV(float u, float v) {

        int x = u * width;
        int y = v * height;

        float nu = static_cast<float>(x) / static_cast<float>(width);
        float nv = static_cast<float>(y) / static_cast<float>(height);

        limitUV(nu, nv);

        return Vector3(nu, nv, 0);
    }

    Vector3 Texture::reverseSphericalMapping(float u, float v) {

        float phi = u * 2 * PIF;
        float theta = v * PIF;

        float px = sycl::cos(phi - PIF);
        float py = -sycl::cos(theta);
        float pz = -sycl::sin(phi - PIF);

        float a = sycl::sqrt(1 - py * py);

        return Vector3(a * px, py, a * pz);
    }
