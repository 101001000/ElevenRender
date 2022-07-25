#pragma once

#include "sycl.h"

#include "Math.hpp"
#include "Ray.h"
#include "Material.h"
#include "Sampling.h"
#include "kernel.h"

/*
 * MIT License
 *
 * Copyright(c) 2019-2021 Asif Ali
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files(the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions :
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */


 // Adaptación del shader de disney de knightcrawler25, derivar en un futuro para aplicar optimizaciones.
 // https://github.com/knightcrawler25/GLSL-PathTracer/blob/master/src/shaders/common/disney.glsl

 // Limitado solo a BRDF sin BSDF

void createBasis(Vector3 normal, Vector3& tangent, Vector3& bitangent);

float SchlickFresnel(float u);

float DielectricFresnel(float cos_theta_i, float eta);

float GTR1(float NDotH, float a);

float GTR2(float NDotH, float a);

float GTR2_aniso(float NDotH, float HDotX, float HDotY, float ax, float ay);

float SmithG_GGX(float NDotV, float alphaG);

float SmithG_GGX_aniso(float NDotV, float VDotX, float VDotY, float ax, float ay);

float powerHeuristic(float a, float b);


SYCL_EXTERNAL float DisneyPdf(Ray ray, HitData& hitdata, Vector3 L);


SYCL_EXTERNAL Vector3 DisneySample(Ray ray, HitData& hitdata, float r1, float r2, float r3);

SYCL_EXTERNAL Vector3 DisneyEval(Ray ray, HitData& hitdata, Vector3 L);
