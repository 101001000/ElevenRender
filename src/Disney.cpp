#include "Disney.h"

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



void createBasis(Vector3 normal, Vector3 &tangent, Vector3 &bitangent) {
    Vector3 UpVector = Vector3(0, 1, 0);
    tangent = Vector3::cross(UpVector, normal).normalized();
    bitangent = Vector3::cross(normal, tangent).normalized();
}

float SchlickFresnel(float u) {
    float m = clamp(1.0 - u, 0.0, 1.0);
    float m2 = m * m;
    return m2 * m2 * m;
}

float DielectricFresnel(float cos_theta_i, float eta) {
    float sinThetaTSq = eta * eta * (1.0f - cos_theta_i * cos_theta_i);

    // Total internal reflection
    if (sinThetaTSq > 1.0)
        return 1.0;

    float cos_theta_t = sqrt(maxf(1.0 - sinThetaTSq, 0.0));

    float rs = (eta * cos_theta_t - cos_theta_i) / (eta * cos_theta_t + cos_theta_i);
    float rp = (eta * cos_theta_i - cos_theta_t) / (eta * cos_theta_i + cos_theta_t);

    return 0.5f * (rs * rs + rp * rp);
}

float GTR1(float NDotH, float a) {
    if (a >= 1.0)
        return (1.0 / PI);
    float a2 = a * a;
    float t = 1.0 + (a2 - 1.0) * NDotH * NDotH;
    return (a2 - 1.0) / (PI * sycl::log(a2) * t);
}

float GTR2(float NDotH, float a) {
    float a2 = a * a;
    float t = 1.0 + (a2 - 1.0) * NDotH * NDotH;
    return a2 / (PI * t * t);
}

float GTR2_aniso(float NDotH, float HDotX, float HDotY, float ax, float ay) {
    float a = HDotX / ax;
    float b = HDotY / ay;
    float c = a * a + b * b + NDotH * NDotH;
    return 1.0 / (PI * ax * ay * c * c);
}

float SmithG_GGX(float NDotV, float alphaG) {
    float a = alphaG * alphaG;
    float b = NDotV * NDotV;
    return 1.0 / (NDotV + sqrt(a + b - a * b));
}

float SmithG_GGX_aniso(float NDotV, float VDotX, float VDotY, float ax, float ay) {
    float a = VDotX * ax;
    float b = VDotY * ay;
    float c = NDotV;
    return 1.0 / (NDotV + sqrt(a * a + b * b + c * c));
}

float powerHeuristic(float a, float b) {
    float t = a * a;
    return t / (b * b + t);
}


float DisneyPdf(Ray ray, HitData& hitdata, Vector3 L) {

    Vector3 N = hitdata.normal;
    Vector3 V = -1 * ray.direction;
    Vector3 H = (L + V).normalized();

    Vector3 T, B;

    createBasis(hitdata.normal, T, B);

    float brdfPdf = 0.0;
    float bsdfPdf = 0.0;

    float NDotH = abs(Vector3::dot(N, H));

    // TODO: Fix importance sampling for microfacet transmission
    if (Vector3::dot(N, L) <= 0.0)
        return 1.0;

    float specularAlpha = maxf(0.001, hitdata.roughness);
    float clearcoatAlpha = lerp(0.1, 0.001, hitdata.clearcoatGloss);

    float diffuseRatio = 0.5 * (1.0 - hitdata.metallic);
    float specularRatio = 1.0 - diffuseRatio;

    float aspect = sqrt(1.0 - hitdata.anisotropic * 0.9);
    float ax = maxf(0.001, hitdata.roughness / aspect);
    float ay = maxf(0.001, hitdata.roughness * aspect);

    // PDFs for brdf
    float pdfGTR2_aniso = GTR2_aniso(NDotH, Vector3::dot(H, T), Vector3::dot(H, B), ax, ay) * NDotH;
    float pdfGTR1 = GTR1(NDotH, clearcoatAlpha) * NDotH;
    float ratio = 1.0 / (1.0 + hitdata.clearcoat);
    float pdfSpec = lerp(pdfGTR1, pdfGTR2_aniso, ratio) / (4.0 * abs(Vector3::dot(L, H)));
    float pdfDiff = abs(Vector3::dot(L, N)) * (1.0 / PI);

    brdfPdf = diffuseRatio * pdfDiff + specularRatio * pdfSpec;

    return brdfPdf;
}


Vector3 DisneySample(Ray ray, HitData& hitdata, float r1, float r2, float r3) {

    Vector3 N = hitdata.normal;
    Vector3 V = -1 * ray.direction;

    Vector3 T;
    Vector3 B;

    createBasis(hitdata.normal, T, B);

    Vector3 dir;

    float diffuseRatio = 0.5 * (1.0 - hitdata.metallic);

    if (r3 < diffuseRatio)
    {
        Vector3 H = CosineSampleHemisphere(r1, r2);
        H = T * H.x + B * H.y + N * H.z;
        dir = H;
    }
    else
    {
        Vector3 H = ImportanceSampleGGX(hitdata.roughness, r1, r2);
        H = T * H.x + B * H.y + N * H.z;
        dir = reflect(-1 * V, H);
    }
    return dir;
}

Vector3 DisneyEval(Ray ray, HitData& hitdata, Vector3 L) {

    Vector3 V = -1 * ray.direction;
    Vector3 H;
    Vector3 T = Vector3();
    Vector3 B = Vector3();

    createBasis(hitdata.normal, T, B);

    H = (L + V).normalized();

    float NDotL = abs(Vector3::dot(hitdata.normal, L));
    float NDotV = abs(Vector3::dot(hitdata.normal, V));
    float NDotH = abs(Vector3::dot(hitdata.normal, H));
    float VDotH = abs(Vector3::dot(V, H));
    float LDotH = abs(Vector3::dot(L, H));

    Vector3 brdf = Vector3(0.0);
    Vector3 bsdf = Vector3(0.0);


    if (hitdata.transmission < 1.0 && Vector3::dot(hitdata.normal, L) > 0.0 && Vector3::dot(hitdata.normal, V) > 0.0)
    {
        Vector3 Cdlin = hitdata.albedo;
        float Cdlum = 0.3 * Cdlin.x + 0.6 * Cdlin.y + 0.1 * Cdlin.z; // luminance approx.

        Vector3 Ctint = Cdlum > 0.0 ? Cdlin / Cdlum : Vector3(1.0f); // normalize lum. to isolate hue+sat
        Vector3 Cspec0 = lerp(hitdata.specular * 0.08 * lerp(Vector3(1.0), Ctint, hitdata.specularTint), Cdlin, hitdata.metallic);
        Vector3 Csheen = lerp(Vector3(1.0), Ctint, hitdata.sheenTint);

        // Diffuse fresnel - go from 1 at normal incidence to .5 at grazing
        // and mix in diffuse retro-reflection based on roughness
        float FL = SchlickFresnel(NDotL);
        float FV = SchlickFresnel(NDotV);
        float Fd90 = 0.5 + 2.0 * LDotH * LDotH * hitdata.roughness;
        float Fd = lerp(1.0, Fd90, FL) * lerp(1.0, Fd90, FV);

        // Based on Hanrahan-Krueger brdf approximation of isotropic bssrdf
        // 1.25 scale is used to (roughly) preserve albedo
        // Fss90 used to "flatten" retroreflection based on roughness
        float Fss90 = LDotH * LDotH * hitdata.roughness;
        float Fss = lerp(1.0, Fss90, FL) * lerp(1.0, Fss90, FV);
        float ss = 1.25 * (Fss * (1.0 / (NDotL + NDotV) - 0.5) + 0.5);

        // TODO: Add anisotropic rotation
        // specular
        float aspect = sqrt(1.0 - hitdata.anisotropic * 0.9);
        float ax = maxf(0.001, hitdata.roughness / aspect);
        float ay = maxf(0.001, hitdata.roughness * aspect);
        float Ds = GTR2_aniso(NDotH, Vector3::dot(H, T), Vector3::dot(H, B), ax, ay);
        float FH = SchlickFresnel(LDotH);
        Vector3 Fs = lerp(Cspec0, Vector3(1.0), FH);
        float Gs = SmithG_GGX_aniso(NDotL, Vector3::dot(L, T), Vector3::dot(L, B), ax, ay);
        Gs *= SmithG_GGX_aniso(NDotV, Vector3::dot(V, T), Vector3::dot(V, B), ax, ay);

        // sheen
        Vector3 Fsheen = FH * hitdata.sheen * Csheen;

        // clearcoat (ior = 1.5 -> F0 = 0.04)
        float Dr = GTR1(NDotH, lerp(0.1, 0.001, hitdata.clearcoatGloss));
        float Fr = lerp(0.04, 1.0, FH);
        float Gr = SmithG_GGX(NDotL, 0.25) * SmithG_GGX(NDotV, 0.25);

        Vector3 p1 = ((1.0 / PI) * lerp(Fd, ss, hitdata.subsurface) * Cdlin + Fsheen) * (1.0 - hitdata.metallic);
        Vector3 p2 = Gs * Fs * Ds;
        float p3 = 0.25 * hitdata.clearcoat * Gr * Fr * Dr;

        brdf = ((1.0 / PI) * lerp(Fd, ss, hitdata.subsurface) * Cdlin + Fsheen) * (1.0 - hitdata.metallic)
            + Gs * Fs * Ds
            + 0.25 * hitdata.clearcoat * Gr * Fr * Dr;

    }

    return brdf;
}
