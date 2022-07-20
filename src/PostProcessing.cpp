#include "PostProcessing.h"

void applyExposure(float* pixels, int width, int height, float exposure) {
	for (int i = 0; i < width * height * 4; i++)
		pixels[i] *= exposure;
}

void flipY(float* pixels, int width, int height) {
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height / 2; y++) {
			float temp[4];
			memcpy(temp, (void*)&pixels[(x + y * width) * 4], sizeof(float) * 4);
			memcpy((void*)&pixels[(x + y * width) * 4], (void*)&pixels[(x + (height - y - 1) * width) * 4], sizeof(float) * 4);
			memcpy((void*)&pixels[(x + (height - y - 1) * width) * 4], (void*)temp, sizeof(float) * 4);
		}
	}
}

void flipX(float* pixels, int width, int height) {
	for (int x = 0; x < width / 2; x++) {
		for (int y = 0; y < height; y++) {
			float temp[4];
			memcpy(temp, (void*)&pixels[(x + y * width) * 4], sizeof(float) * 4);
			memcpy((void*)&pixels[(x + y * width) * 4], (void*)&pixels[((width - x - 1) + y * width) * 4], sizeof(float) * 4);
			memcpy((void*)&pixels[((width - x - 1) + y * width) * 4], (void*)temp, sizeof(float) * 4);
		}
	}
}

void clampPixels(float* pixels, int width, int height) {
	for (int i = 0; i < width * height * 4; i++)
		pixels[i] = clamp(pixels[i], 0, 1);
}

void reinhardTonemap(float* pixels, int width, int height) {

	for (int i = 0; i < width * height * 4; i++)
		pixels[i] = pixels[i] / (1.0f + pixels[i]);
}

void acesTonemap(float* pixels, int width, int height) {

	float a = 2.51f;
	float b = 0.03f;
	float c = 2.43f;
	float d = 0.59f;
	float e = 0.14f;

	for (int i = 0; i < width * height * 4; i++)
		pixels[i] = clamp((pixels[i] * 0.6 * (a * pixels[i] * 0.6 + b)) / (pixels[i] * 0.6 * (c * pixels[i] * 0.6 + d) + e), 0.0f, 1.0f);

}

void applysRGB(float* pixels, int width, int height) {
	for (int i = 0; i < width * height * 4; i++)
		pixels[i] = pow(pixels[i], 1.0 / 2.2);
}

float gaussianDist(float x, float sigma) {
	return 0.39894 * exp(-0.5 * x * x / (sigma * sigma)) / sigma;
}

void getThreshold(float* pixels, int width, int height, float threshold, float* result) {

	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {

			float v = 0;

			v += pixels[(x + y * width) * 4 + 0] / 3.0;
			v += pixels[(x + y * width) * 4 + 1] / 3.0;
			v += pixels[(x + y * width) * 4 + 2] / 3.0;

			if (v < threshold) {
				result[(x + y * width) * 4 + 0] = 0;
				result[(x + y * width) * 4 + 1] = 0;
				result[(x + y * width) * 4 + 2] = 0;
				result[(x + y * width) * 4 + 2] = 0;
			}
			else {
				result[(x + y * width) * 4 + 0] = pixels[(x + y * width) * 4 + 0];
				result[(x + y * width) * 4 + 1] = pixels[(x + y * width) * 4 + 1];
				result[(x + y * width) * 4 + 2] = pixels[(x + y * width) * 4 + 2];
				result[(x + y * width) * 4 + 3] = pixels[(x + y * width) * 4 + 3];
			}
		}
	}

}

void gaussianBlur(float* pixels, int width, int height, int kernelSize, float* result) {

	float* resultTemp = new float[4 * width * height];

	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {

			float sum[4] = { 0, 0, 0, 0 };

			for (int xk = 0; xk < kernelSize; xk++) {
				for (int yk = 0; yk < kernelSize; yk++) {

					float t1 = 1.0 / ((2.0 * PI) * ((float)kernelSize * (float)kernelSize));
					float tx = xk - (int)(kernelSize / 2.0);
					float ty = yk - (int)(kernelSize / 2.0);
					float t2 = -((tx * tx) + (ty * ty)) / (2.0 * (float)kernelSize * (float)kernelSize);

					float w = t1 * exp(t2);

					int xx = clamp(x + tx, 0, width - 1);
					int yy = clamp(y + ty, 0, height - 1);

					sum[0] += w * pixels[(xx + yy * width) * 4 + 0];
					sum[1] += w * pixels[(xx + yy * width) * 4 + 1];
					sum[2] += w * pixels[(xx + yy * width) * 4 + 2];
				}
			}

			resultTemp[(x + y * width) * 4 + 0] = sum[0];
			resultTemp[(x + y * width) * 4 + 1] = sum[1];
			resultTemp[(x + y * width) * 4 + 2] = sum[2];
			resultTemp[(x + y * width) * 4 + 3] = 1;
		}
	}

	for (int i = 0; i < width * height * 4; i++)
		result[i] = resultTemp[i];

	delete(resultTemp);
}

void basicBlur(float* pixels, int width, int height, float threshold, float power, float radius) {

	int blurSize = radius / 2;
	float sd = radius;
	float kernelSize = (blurSize * 2 + 1);

	float* blurMatrix = new float[4 * width * height];
	float* thresholdMatrix = new float[4 * width * height];
	float* kernel = new float[kernelSize * kernelSize];


	for (int x = 0; x < kernelSize; x++) {
		for (int y = 0; y < kernelSize; y++) {

			float t1 = 1.0 / ((2.0 * PI) * (sd * sd));
			float tx = x - (int)(kernelSize / 2.0);
			float ty = y - (int)(kernelSize / 2.0);
			float t2 = -((tx * tx) + (ty * ty)) / (2.0 * sd * sd);

			kernel[(int)(x + y * kernelSize)] = t1 * exp(t2);
		}
	}

	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {

			float v = 0;

			v += pixels[(x + y * width) * 4 + 0] / 3.0;
			v += pixels[(x + y * width) * 4 + 1] / 3.0;
			v += pixels[(x + y * width) * 4 + 2] / 3.0;

			if (v < threshold) {
				thresholdMatrix[(x + y * width) * 4 + 0] = 0;
				thresholdMatrix[(x + y * width) * 4 + 1] = 0;
				thresholdMatrix[(x + y * width) * 4 + 2] = 0;
				thresholdMatrix[(x + y * width) * 4 + 2] = 0;
			}
			else {
				thresholdMatrix[(x + y * width) * 4 + 0] = pixels[(x + y * width) * 4 + 0];
				thresholdMatrix[(x + y * width) * 4 + 1] = pixels[(x + y * width) * 4 + 1];
				thresholdMatrix[(x + y * width) * 4 + 2] = pixels[(x + y * width) * 4 + 2];
				thresholdMatrix[(x + y * width) * 4 + 3] = pixels[(x + y * width) * 4 + 3];
			}
		}
	}


	for (int x = blurSize; x < width - blurSize; x++) {
		for (int y = blurSize; y < height - blurSize; y++) {

			float v[4] = { 0,0,0,0 };

			int ii = 0;

			for (int i = x - blurSize; i <= x + blurSize; i++) {

				int jj = 0;

				for (int j = y - blurSize; j <= y + blurSize; j++) {
					v[0] += thresholdMatrix[(i + j * width) * 4 + 0] * kernel[(int)(ii + jj * kernelSize)];
					v[1] += thresholdMatrix[(i + j * width) * 4 + 1] * kernel[(int)(ii + jj * kernelSize)];
					v[2] += thresholdMatrix[(i + j * width) * 4 + 2] * kernel[(int)(ii + jj * kernelSize)];
					v[3] += thresholdMatrix[(i + j * width) * 4 + 3] * kernel[(int)(ii + jj * kernelSize)];
					jj++;
				}
				ii++;
			}

			blurMatrix[(x + y * width) * 4 + 0] = v[0];
			blurMatrix[(x + y * width) * 4 + 1] = v[1];
			blurMatrix[(x + y * width) * 4 + 2] = v[2];
			blurMatrix[(x + y * width) * 4 + 3] = v[3];
		}
	}

	for (int i = 0; i < width * height * 4; i++)
		pixels[i] += blurMatrix[i] * power;

}

void downscale(float* pixels, int width, int height, int nWidth, int nheight, float* result) {

	float rx = width / nWidth;
	float ry = height / nheight;

	for (int x = 0; x < nWidth; x++) {
		for (int y = 0; y < nheight; y++) {
			result[4 * (y * nWidth + x) + 0] = pixels[4 * ((int)(y * ry * width) + (int)(x * rx)) + 0];
			result[4 * (y * nWidth + x) + 1] = pixels[4 * ((int)(y * ry * width) + (int)(x * rx)) + 1];
			result[4 * (y * nWidth + x) + 2] = pixels[4 * ((int)(y * ry * width) + (int)(x * rx)) + 2];
			result[4 * (y * nWidth + x) + 3] = 1;
		}
	}
}

void upscale(float* pixels, int width, int height, int nWidth, int nheight, float* result) {

	float rx = width / nWidth;
	float ry = height / nheight;

	for (int x = 0; x < nWidth; x++) {
		for (int y = 0; y < nheight; y++) {
			result[4 * (y * nWidth + x) + 0] = pixels[4 * ((int)(y * ry * width) + (int)(x * rx)) + 0];
			result[4 * (y * nWidth + x) + 1] = pixels[4 * ((int)(y * ry * width) + (int)(x * rx)) + 1];
			result[4 * (y * nWidth + x) + 2] = pixels[4 * ((int)(y * ry * width) + (int)(x * rx)) + 2];
			result[4 * (y * nWidth + x) + 3] = 1;
		}
	}
}

void beautyBloom(float* pixels, int width, int height, float threshold, float power, float radius) {

	int nw = 640;
	int nh = 360;

	float* pixelsDown = new float[nw * nh * 4];

	downscale(pixels, width, height, nw, nh, pixelsDown);
	upscale(pixelsDown, nw, nh, width, height, pixels);

	/*

	float bloomFactors[] = { 1.0, 0.8, 0.6, 0.4, 0.2 };

	int kernelSizes[] = { 3, 5, 7, 9, 11 };

	int nW = width;
	int nH = height;

	for (int i = 0; i < 5; i++) {

		float* pixelsDown = new float[nW*nH*4];
		float* pixelsUp = new float[width * height * 4];
		float* thresholdMatrix = new float[nW * nH * 4];
		float* blurMatrix = new float[nW * nH * 4];

		downscale(pixels, width, height, nW, nH, pixelsDown);
		getThreshold(pixelsDown, nW, nH, threshold, thresholdMatrix);
		gaussianBlur(thresholdMatrix, nW, nH, kernelSizes[i], blurMatrix);
		upscale(pixels, nW, nH, width, height, pixelsUp);

		for (int j = 0; j < width * height; j++) {

			float w = power * lerp(bloomFactors[i], 1.2 - bloomFactors[i], radius);

			pixels[j * 4 + 0] += w * pixelsUp[j * 4 + 0];
			pixels[j * 4 + 1] += w * pixelsUp[j * 4 + 1];
			pixels[j * 4 + 2] += w * pixelsUp[j * 4 + 2];
			pixels[j * 4 + 3] += w * pixelsUp[j * 4 + 3];
		}

		delete(pixelsDown);
		delete(pixelsUp);
		delete(thresholdMatrix);
		delete(blurMatrix);
	}
	*/
}

void HDRtoLDR(float* pixelsIn, unsigned char* pixelsOut, int width, int height) {
	for (int i = 0; i < width * height * 3; i++) {
		if (pixelsIn[i] > 1)
			pixelsIn[i] = 1;
		pixelsOut[i] = (unsigned char)(pixelsIn[i] * 255);
	}
}