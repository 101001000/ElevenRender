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

SYCL_EXTERNAL Vector3 DisneyEval(HitData& hd, Vector3 V, Vector3 N, Vector3 L);
SYCL_EXTERNAL Vector3 DisneySample(HitData& hd, Vector3 V, Vector3 N, float r1, float r2, float r3);
SYCL_EXTERNAL float DisneyPdf(HitData& hitdata, Vector3 V, Vector3 N, Vector3 L);
