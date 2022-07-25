#pragma once


#include "Texture.hpp"
#include "Sampling.h"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#pragma clang diagnostic ignored "-Winconsistent-dllimport"
#include <CL/sycl.hpp>
#pragma clang diagnostic pop


class HDRI {

public:

	Texture texture;
	float* cdf;
	float radianceSum = 0;

	HDRI();

	HDRI(Vector3 color);

#if !defined(__CUDACC__)

	HDRI(std::string filepath);
#else

#endif

	inline void generateCDF2();

	inline void generateCDF();

	int binarySearch(float* arr, float value, int length);


	SYCL_EXTERNAL float pdf(int x, int y);

	SYCL_EXTERNAL Vector3 sample(float r1);

	inline Vector3 sample2(float r1);

};
