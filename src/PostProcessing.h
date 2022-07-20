#pragma once

#include "Math.hpp"

void applyExposure(float* pixels, int width, int height, float exposure);

void flipY(float* pixels, int width, int height);

void flipX(float* pixels, int width, int height);

void clampPixels(float* pixels, int width, int height);

void reinhardTonemap(float* pixels, int width, int height);

void acesTonemap(float* pixels, int width, int height);

void applysRGB(float* pixels, int width, int height);

float gaussianDist(float x, float sigma);

void getThreshold(float* pixels, int width, int height, float threshold, float* result);

void gaussianBlur(float* pixels, int width, int height, int kernelSize, float* result);

void basicBlur(float* pixels, int width, int height, float threshold, float power, float radius);

void downscale(float* pixels, int width, int height, int nWidth, int nheight, float* result);

void upscale(float* pixels, int width, int height, int nWidth, int nheight, float* result);

void beautyBloom(float* pixels, int width, int height, float threshold, float power, float radius);

void HDRtoLDR(float* pixelsIn, unsigned char* pixelsOut, int width, int height);
