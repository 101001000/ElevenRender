#include "HDRI.h"


HDRI::HDRI() {
	texture = Texture();
}

HDRI::HDRI(Vector3 color) {

	// Esto debería hacerlo con constructor
	texture.data = new float[1024 * 1024 * 3];
	texture.width = 1024;
	texture.height = 1024;

	for (int i = 0; i < texture.width * texture.height; i++) {
		texture.data[3 * i + 0] = color.x;
		texture.data[3 * i + 1] = color.y;
		texture.data[3 * i + 2] = color.z;
	}

	cdf = new float[texture.width * texture.height + 1];

	generateCDF();
}

#if !defined(__CUDACC__)

HDRI::HDRI(std::string filepath) {

	int channels;

	float* tmp_data = stbi_loadf(filepath.c_str(), &texture.width, &texture.height, &channels, 0);

	texture.data = new float[texture.width * texture.height * 3];
	texture.filter = Texture::Filter::BILINEAR;

	//std::cout << texture.width << " . " << texture.height << std::endl;

	for (int i = 0; i < texture.width * texture.height * 3; i++)
		texture.data[i] = tmp_data[i];

	texture.xOffset = 0.5;

	stbi_image_free(tmp_data);

	cdf = new float[texture.width * texture.height + 1];

	generateCDF();

	printf("HDRI %s loaded\n", filepath.c_str());
}
#else

#endif

inline void HDRI::generateCDF2() {

	for (int y = 0; y < texture.height; y++) {
		for (int x = 0; x < texture.width; x++) {

			float r = 0;
			float u, v;

			Vector3 sample = uniformSampleSphere((float)x / (float)texture.width, (float)y / (float)texture.height);

			Texture::sphericalMapping(Vector3(), sample, 1, u, v);

			r += texture.getValueFromUVFiltered(u, v).x;
			r += texture.getValueFromUVFiltered(u, v).y;
			r += texture.getValueFromUVFiltered(u, v).z;

			radianceSum += r;
		}
	}

	for (int y = 0; y < texture.height; y++) {
		for (int x = 0; x < texture.width; x++) {

			float r = 0;

			float u, v;

			Vector3 sample = uniformSampleSphere((float)x / (float)texture.width, (float)y / (float)texture.height);

			Texture::sphericalMapping(Vector3(), sample, 1, u, v);

			r += texture.getValueBilinear(u, v).x;
			r += texture.getValueBilinear(u, v).y;
			r += texture.getValueBilinear(u, v).z;

			r /= radianceSum;

			cdf[y * texture.width + x + 1] = r + cdf[y * texture.width + x];
		}
	}
}

inline void HDRI::generateCDF() {

	int c = 0;
	radianceSum = 0;
	cdf[0] = 0;

	// Total radiance of the HDRI
	for (int j = 0; j < texture.height; j++) {
		for (int i = 0; i < texture.width; i++) {
			Vector3 data = texture.getValueFromCoordinates(i, j);
			radianceSum += data.x + data.y + data.z;
		}
	}
	// CDF calculation
	for (int j = 0; j < texture.height; j++) {
		for (int i = 0; i < texture.width; i++) {
			Vector3 data = texture.getValueFromCoordinates(i, j);
			cdf[c + 1] = cdf[c] + (data.x + data.y + data.z) / radianceSum;
			c++;
		}
	}
}

int HDRI::binarySearch(float* arr, float value, int length) {

	int from = 0;
	int to = length - 1;

	while (to - from > 0) {
		int m = from + (to - from) / 2;
		if (value == arr[m]) return m;
		if (value < arr[m])	to = m - 1;
		if (value > arr[m]) from = m + 1;
	}
	return to;
}


float HDRI::pdf(int x, int y) {

	Vector3 dv = texture.getValueFromCoordinates(x, y);
	float theta = (((float)y / (float)texture.height)) * PI;

	// Semisphere area
	return ((dv.x + dv.y + dv.z) / radianceSum) * texture.width * texture.height / (2.0 * PI * sycl::sin(theta));
}

Vector3 HDRI::sample(float r1) {

	int count = binarySearch(cdf, r1, texture.width * texture.height);

	int x = count % texture.width;
	int y = count / texture.width;

	return Vector3(x, y, 0);
}

inline Vector3 HDRI::sample2(float r1) {

	int count = binarySearch(cdf, r1, texture.width * texture.height);

	int wu = count % texture.width;
	int wv = count / texture.width;

	float u, v;

	Vector3 sample = uniformSampleSphere((float)wu / (float)texture.width, (float)wv / (float)texture.height);

	Texture::sphericalMapping(Vector3(), sample, 1, u, v);

	return Vector3(u * texture.width, v * texture.height, count);
}
