﻿#include "kernel.h"

#include <math.h>
#include <stdio.h>

#include <CL/sycl.hpp>
#include <iostream>
#include <limits>

#include "BVH.h"
#include "Camera.h"
#include "Definitions.h"
#include "Disney.h"
#include "HDRI.h"
#include "Hit.hpp"
#include "Material.hpp"
#include "Math.hpp"
#include "PointLight.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "libs/stb_image.h"

// TODO HACER ESTO CON MEMORIA DINÁMICA PARA ELIMINAR EL MÁXIMO DE 1920*1080

RngGenerator::RngGenerator(uint32_t _seed) {
    this->seed = _seed + 1;
    for (int i = 0; i < _seed; i++) this->next();
}

float RngGenerator::next() {
    seed ^= seed << 13;
    seed ^= seed >> 17;
    seed ^= seed << 5;
    return ((float)seed / (float)UINT_MAX);
}

Passes parsePass(std::string pass) {

    std::transform(pass.begin(), pass.end(), pass.begin(), ::tolower);

    if (pass.compare("beauty") == 0)
        return BEAUTY;
    if (pass.compare("denoise") == 0)
        return BITANGENT;
    if (pass.compare("normal") == 0)
        return NORMAL;
    if (pass.compare("tangent") == 0)
        return TANGENT;
    if (pass.compare("bitangent") == 0)
        return BITANGENT;

    return BEAUTY;
}

unsigned long textureMemory = 0;
unsigned long geometryMemory = 0;

void generateHitData(dev_Scene* dev_scene_g, Material* material,
                     HitData& hitdata, Hit hit) {
    Vector3 tangent, bitangent, normal;

    normal = hit.normal;
    tangent = hit.tangent;
    bitangent = hit.bitangent;

    if (material->albedoTextureID < 0) {
        hitdata.albedo = material->albedo;
    } else {
        hitdata.albedo = dev_scene_g->textures[material->albedoTextureID]
                             .getValueFromUVFiltered(hit.tu, hit.tv);
    }

    if (material->emissionTextureID < 0) {
        hitdata.emission = material->emission;
    } else {
        hitdata.emission = dev_scene_g->textures[material->emissionTextureID]
                               .getValueFromUVFiltered(hit.tu, hit.tv);
    }

    if (material->roughnessTextureID < 0) {
        hitdata.roughness = material->roughness;
    } else {
        hitdata.roughness = dev_scene_g->textures[material->roughnessTextureID]
                                .getValueFromUVFiltered(hit.tu, hit.tv)
                                .x;
    }

    if (material->metallicTextureID < 0) {
        hitdata.metallic = material->metallic;
    } else {
        hitdata.metallic = dev_scene_g->textures[material->metallicTextureID]
                               .getValueFromUVFiltered(hit.tu, hit.tv)
                               .x;
    }

    if (material->normalTextureID < 0) {
        hitdata.normal = normal;
    } else {
        Vector3 ncolor =
            dev_scene_g->textures[material->normalTextureID].getValueFromUV(
                hit.tu, hit.tv);
        Vector3 localNormal = (ncolor * 2) - 1;
        Vector3 worldNormal =
            (localNormal.x * tangent - localNormal.y * bitangent +
             localNormal.z * normal)
                .normalized();

        hitdata.normal = worldNormal;
    }

    // Convert linear to sRGB
    hitdata.roughness = sycl::pow(hitdata.roughness, 2.2f);
    hitdata.metallic = sycl::pow(hitdata.metallic, 2.2f);

    hitdata.clearcoatGloss = material->clearcoatGloss;
    hitdata.clearcoat = material->clearcoat;
    hitdata.anisotropic = material->anisotropic;
    hitdata.eta = material->eta;
    hitdata.transmission = material->transmission;
    hitdata.specular = material->specular;
    hitdata.specularTint = material->specularTint;
    hitdata.sheenTint = material->sheenTint;
    hitdata.subsurface = material->subsurface;
    hitdata.sheen = material->sheen;

    hitdata.tangent = tangent;
    hitdata.bitangent = bitangent;
}

void setupKernel(dev_Scene* dev_scene_g, int idx, sycl::stream out) {
    
    dev_scene_g->dev_randstate[idx] = RngGenerator(idx);

    for (int i = 0; i < PASSES_COUNT; i++) {
        dev_scene_g->dev_passes[(i * dev_scene_g->camera->xRes * dev_scene_g->camera->yRes *
                    4) +
                   (4 * idx + 0)] = 0.0;
        dev_scene_g->dev_passes[(i * dev_scene_g->camera->xRes * dev_scene_g->camera->yRes *
                    4) +
                   (4 * idx + 1)] = 0.0;
        dev_scene_g->dev_passes[(i * dev_scene_g->camera->xRes * dev_scene_g->camera->yRes *
                    4) +
                   (4 * idx + 2)] = 0.0;
        dev_scene_g->dev_passes[(i * dev_scene_g->camera->xRes * dev_scene_g->camera->yRes *
                    4) +
                   (4 * idx + 3)] = 1.0;
    }

    dev_scene_g->dev_samples[idx] = 0;

    if (idx == 0) {
        int triSum = 0;
        for (int i = 0; i < dev_scene_g->meshObjectCount; i++) {
            dev_scene_g->meshObjects[i].tris += triSum;
            triSum += dev_scene_g->meshObjects[i].triCount;
        }
    }
}

Hit throwRay(Ray ray, dev_Scene* scene) {
    Hit nearestHit = Hit();

#if USEBVH
    scene->bvh->transverse(ray, nearestHit);
#else:
    for (int j = 0; j < scene->meshObjectCount; j++) {
        Hit hit = Hit();

        if (scene->meshObjects[j].hit(ray, hit)) {
            if (!nearestHit.valid) nearestHit = hit;

            if ((hit.position - ray.origin).length() <
                (nearestHit.position - ray.origin).length())
                nearestHit = hit;
        }
    }
#endif
    return nearestHit;
}

Vector3 pointLight(Ray ray, HitData hitdata, dev_Scene* scene, Vector3 point,
                   float& pdf, float r1) {
    if (scene->pointLightCount <= 0) {
        pdf = 0;
        return Vector3::Zero();
    }

    pdf = ((float)scene->pointLightCount) / (2.0 * PI);

    // Retrieve a random light
    PointLight light = scene->pointLights[(int)(scene->pointLightCount * r1)];

    Vector3 newDir = (light.position - point).normalized();

    float dist = (light.position - point).length();

    // Test if the point is visible from the light
    Ray shadowRay(point + newDir * 0.001, newDir);
    Hit shadowHit = throwRay(shadowRay, scene);
    float shadowDist = (shadowHit.position - point).length();

    if (shadowHit.valid && shadowDist < dist) return Vector3();

    // Quadratic attenuation
    Vector3 pointLightValue = (light.radiance / (dist * dist));

    Vector3 brdfDisney = DisneyEval(ray, hitdata, newDir);

    return pointLightValue * brdfDisney *
           abs(Vector3::dot(newDir, hitdata.normal)) / pdf;
}

// Sampling HDRI
// The main idea is to get a random point of the HDRI, weighted by their
// importance and then get the direction from the center to that point as if
// that pixel would be in a sphere of infinite radius.
Vector3 hdriLight(Ray ray, dev_Scene* scene, Vector3 point, HitData hitdata,
                  float r1, float r2, float r3, float& pdf) {
    if (!HDRIIS) {
        Vector3 newDir = uniformSampleSphere(r1, r2).normalized();

        float u, v;

        Texture::sphericalMapping(Vector3(), -1 * newDir, 1, u, v);

        Ray shadowRay(point + newDir * 0.001, newDir);

        Hit shadowHit = throwRay(shadowRay, scene);

        if (shadowHit.valid) return Vector3();

        Vector3 hdriValue = scene->hdri->texture.getValueFromUV(u, v);

        Vector3 brdfDisney = DisneyEval(ray, hitdata, newDir);

        pdf = (1.0 / (2.0 * PI * PI));

        return brdfDisney * abs(Vector3::dot(newDir, hitdata.normal)) *
               hdriValue / (1.0 / (2.0 * PI));
    } else {
        Vector3 textCoordinate = scene->hdri->sample(r1);

        float nu = textCoordinate.x / (float)scene->hdri->texture.width;
        float nv = textCoordinate.y / (float)scene->hdri->texture.height;

        float iu = scene->hdri->texture.inverseTransformUV(nu, nv).x;
        float iv = scene->hdri->texture.inverseTransformUV(nu, nv).y;

        Vector3 newDir =
            -scene->hdri->texture.reverseSphericalMapping(iu, iv).normalized();

        Ray shadowRay(point + newDir * 0.001, newDir);
        Hit shadowHit = throwRay(shadowRay, scene);
        if (shadowHit.valid) return Vector3();

        Vector3 hdriValue = scene->hdri->texture.getValueFromUV(iu, iv);

        Vector3 brdfDisney = DisneyEval(ray, hitdata, newDir);

        pdf = scene->hdri->pdf(iu * scene->hdri->texture.width,
                               iv * scene->hdri->texture.height);

        return brdfDisney * abs(Vector3::dot(newDir, hitdata.normal)) *
               hdriValue / pdf;
    }
}

void calculateCameraRay(int x, int y, Camera& camera, Ray& ray, float r1,
                        float r2, float r3, float r4, float r5) {
    // Relative coordinates for the point where the first ray will be launched
    float dx = camera.position.x +
               ((float)x) / ((float)camera.xRes) * camera.sensorWidth;
    float dy = camera.position.y +
               ((float)y) / ((float)camera.yRes) * camera.sensorHeight;

    // Absolute coordinates for the point where the first ray will be launched
    float odx = (-camera.sensorWidth / 2.0) + dx;
    float ody = (-camera.sensorHeight / 2.0) + dy;

    // Random part of the sampling offset so we get antialasing
    float rx = (1.0 / (float)camera.xRes) * (r1 - 0.5) * camera.sensorWidth;
    float ry = (1.0 / (float)camera.yRes) * (r2 - 0.5) * camera.sensorHeight;

    // Sensor point, the point where intersects the ray with the sensor
    float SPx = odx + rx;
    float SPy = ody + ry;
    float SPz = camera.position.z + camera.focalLength;

    /*
    | 1     0           0| |x|   |        x         |   |x'|
    | 0   cos θ    −sin θ| |y| = | y cos θ − z sin θ| = |y'|
    | 0   sin θ     cos θ| |z|   |y sin θ + z cos θ |   |z'|
    */

    /*
    | cos θ    0    sin θ| |x|   | x cos θ + z sin θ|   |x'|
    | 0        1        0| |y| = | y                | = |y'|
    | -sin θ   0    cos θ| |z|   | z cos θ - x sin θ|   |z'|
    */

    /*
    |cos θ   −sin θ   0| |x|   |x cos θ − y sin θ|   |x'|
    |sin θ    cos θ   0| |y| = |x sin θ + y cos θ| = |y'|
    |  0       0      1| |z|   |        z        |   |z'|
    */

    // XYZ Eulers's Rotation
    // TODO CLEANUP AND PRECALC
    Vector3 rotation = camera.rotation;

    rotation *= (PI / 180.0);

    Vector3 dir = Vector3(SPx, SPy, SPz) - camera.position;
    Vector3 dirXRot = Vector3(
        dir.x, dir.y * sycl::cos(rotation.x) - dir.z * sycl::sin(rotation.x),
        dir.y * sycl::sin(rotation.x) + dir.z * sycl::cos(rotation.x));
    Vector3 dirYRot = Vector3(
        dirXRot.x * sycl::cos(rotation.y) + dirXRot.z * sycl::sin(rotation.y),
        dirXRot.y,
        dirXRot.z * sycl::cos(rotation.y) - dirXRot.x * sycl::sin(rotation.y));
    Vector3 dirZRot = Vector3(
        dirYRot.x * sycl::cos(rotation.z) - dirYRot.y * sycl::sin(rotation.z),
        dirYRot.x * sycl::sin(rotation.z) + dirYRot.y * sycl::cos(rotation.z),
        dirYRot.z);

    // The initial ray is created from the camera position to the sensor point.
    // No rotation is taken into account.
    ray = Ray(camera.position, dirZRot);

    if (camera.bokeh) {
        float rIPx, rIPy;

        // The diameter of the camera iris
        float diameter = camera.focalLength / camera.aperture;

        // Total length from the camera to the focus plane
        float l = camera.focusDistance + camera.focalLength;

        // The point from the initial ray which is actually in focus
        // Vector3 focusPoint = ray.origin + ray.direction * (l /
        // (ray.direction.z));
        // Mala aproximación, encontrar soluición
        Vector3 focusPoint = ray.origin + ray.direction * l;

        // Sampling for the iris of the camera
        uniformCircleSampling(r3, r4, r5, rIPx, rIPy);

        rIPx *= diameter * 0.5;
        rIPy *= diameter * 0.5;

        Vector3 orig = camera.position + Vector3(rIPx, rIPy, 0);

        // Blurred ray
        ray = Ray(orig, focusPoint - orig);
    }
}

void shade(dev_Scene& scene, Ray& ray, HitData& hitdata, Hit& nearestHit,
           Vector3& newDir, float r1, float r2, float r3, Vector3& hitLight,
           Vector3& reduction, sycl::stream out, int idx) {


    Vector3 brdfDisney = DisneyEval(ray, hitdata, newDir);

    float brdfPdf = DisneyPdf(ray, hitdata, newDir);
    float hdriPdf;
    float pointPdf = 0;

    Vector3 hdriLightCalc = hdriLight(ray, &scene, nearestHit.position, hitdata,
                                      r1, r2, r3, hdriPdf);
    Vector3 pointLightCalc =
        pointLight(ray, hitdata, &scene, nearestHit.position, pointPdf, r1);
    Vector3 brdfLightCalc =
        hitdata.emission *
        (brdfDisney * abs(Vector3::dot(newDir, hitdata.normal))) / brdfPdf;

    float w1 = hdriPdf / (hdriPdf + pointPdf + brdfPdf);
    float w2 = pointPdf / (hdriPdf + pointPdf + brdfPdf);
    float w3 = brdfPdf / (hdriPdf + pointPdf + brdfPdf);

    reduction = Vector3::One();

    hitLight = reduction *
               (w1 * hdriLightCalc + w2 * pointLightCalc + 1 * brdfLightCalc);

    reduction *=
        (brdfDisney * abs(Vector3::dot(newDir, hitdata.normal))) / brdfPdf;
        
}

void calculateBounce(Ray& incomingRay, HitData& hitdata, Vector3& bouncedDir,
                     float r1, float r2, float r3) {
    bouncedDir = DisneySample(incomingRay, hitdata, r1, r2, r3);
}

void renderingKernel(dev_Scene* scene, int idx, sycl::stream out) {

    RngGenerator rnd = scene->dev_randstate[idx];

    unsigned int sa = scene->dev_samples[idx];

    Ray ray;

    int x = idx % scene->camera->xRes;
    int y = scene->camera->yRes - (idx / scene->camera->xRes);


    calculateCameraRay(x, y, *scene->camera, ray, rnd.next(), rnd.next(),
                       rnd.next(), rnd.next(), rnd.next());

    // Accumulated radiance
    Vector3 light = Vector3::Zero();

    // Accumulated radiance
    Vector3 normal = Vector3::Zero();
    Vector3 tangent = Vector3::Zero();
    Vector3 bitangent = Vector3::Zero();

    // How much light is lost in the path
    Vector3 reduction = Vector3::One();

    int i = 0;

    for (i = 0; i < MAXBOUNCES; i++) {

        Vector3 hitLight;
        HitData hitdata;
        Vector3 bouncedDir;

        int materialID = 0;

        Hit nearestHit = throwRay(ray, scene);

        if (!nearestHit.valid) {
            float u, v;
            Texture::sphericalMapping(Vector3(), -1 * ray.direction, 1, u, v);
            light +=
                scene->hdri->texture.getValueFromUVFiltered(u, v) * reduction;
            break;
        }

        materialID = scene->meshObjects[nearestHit.objectID].materialID;

        Material* material = &scene->materials[materialID];

        generateHitData(scene, material, hitdata, nearestHit);

        calculateBounce(ray, hitdata, bouncedDir, rnd.next(), rnd.next(),
                        rnd.next());

        shade(*scene, ray, hitdata, nearestHit, bouncedDir, rnd.next(), rnd.next(), rnd.next(), hitLight, reduction, out, idx);

        light += hitLight;

        // First hit
        if (i == 0) {
            normal = nearestHit.normal;
            tangent = nearestHit.tangent;
            bitangent = nearestHit.bitangent;
        }

        ray = Ray(nearestHit.position + bouncedDir * 0.001, bouncedDir);
    }

    // dev_pathcount[idx] += i;

    light = clamp(light, 0, 10);

    if (!sycl::isnan(light.x) && !sycl::isnan(light.y) &&
        !sycl::isnan(light.z)) {
        if (sa > 0) {
            for (int pass = 0; pass < PASSES_COUNT; pass++) {
                if (pass != DENOISE) {
                    scene->dev_passes[(pass * 1920 * 1080 * 4) + (4 * idx + 0)] *=
                        ((float)sa) / ((float)(sa + 1));
                    scene->dev_passes[(pass * 1920 * 1080 * 4) + (4 * idx + 1)] *=
                        ((float)sa) / ((float)(sa + 1));
                    scene->dev_passes[(pass * 1920 * 1080 * 4) + (4 * idx + 2)] *=
                        ((float)sa) / ((float)(sa + 1));
                }
            }
        }

        scene->dev_passes[(BEAUTY * 1920 * 1080 * 4) + (4 * idx + 0)] +=
            light.x / ((float)sa + 1);
        scene->dev_passes[(BEAUTY * 1920 * 1080 * 4) + (4 * idx + 1)] +=
            light.y / ((float)sa + 1);
        scene->dev_passes[(BEAUTY * 1920 * 1080 * 4) + (4 * idx + 2)] +=
            light.z / ((float)sa + 1);

        scene->dev_passes[(NORMAL * 1920 * 1080 * 4) + (4 * idx + 0)] +=
            normal.x / ((float)sa + 1);
        scene->dev_passes[(NORMAL * 1920 * 1080 * 4) + (4 * idx + 1)] +=
            normal.y / ((float)sa + 1);
        scene->dev_passes[(NORMAL * 1920 * 1080 * 4) + (4 * idx + 2)] +=
            normal.z / ((float)sa + 1);

        scene->dev_passes[(TANGENT * 1920 * 1080 * 4) + (4 * idx + 0)] +=
            tangent.x / ((float)sa + 1);
        scene->dev_passes[(TANGENT * 1920 * 1080 * 4) + (4 * idx + 1)] +=
            tangent.y / ((float)sa + 1);
        scene->dev_passes[(TANGENT * 1920 * 1080 * 4) + (4 * idx + 2)] +=
            tangent.z / ((float)sa + 1);

        scene->dev_passes[(BITANGENT * 1920 * 1080 * 4) + (4 * idx + 0)] +=
            bitangent.x / ((float)sa + 1);
        scene->dev_passes[(BITANGENT * 1920 * 1080 * 4) + (4 * idx + 1)] +=
            bitangent.y / ((float)sa + 1);
        scene->dev_passes[(BITANGENT * 1920 * 1080 * 4) + (4 * idx + 2)] +=
            bitangent.z / ((float)sa + 1);

        scene->dev_samples[idx]++;
    }

    scene->dev_randstate[idx] = rnd;
}

int renderSetup(sycl::queue& q, Scene* scene, dev_Scene* dev_scene) {
    printf("Initializing rendering... \n");

    //dev_passes = sycl::malloc_device<float>(1920*1080*4*PASSES_COUNT, q);

    unsigned int meshObjectCount = scene->meshObjectCount();
    unsigned int triCount = scene->triCount();

    Camera* camera = scene->getMainCamera();

    MeshObject* meshObjects = scene->getMeshObjects();

    Tri* tris = scene->getTris();
    BVH* bvh = scene->buildBVH();

    printf("End building BVH");

    printf("Allocating GPU memory");

    float* dev_pixelBuffer =
        sycl::malloc_device<float>(camera->xRes * camera->yRes * 4, q);

    for (int i = 0; i < PASSES_COUNT; i++) {
        float* d_pass =
            sycl::malloc_device<float>(camera->xRes * camera->yRes * 4, q);
        q.memcpy(&(dev_scene->dev_passes[i]), &(d_pass), sizeof(float*));
    }

    Camera* dev_camera = sycl::malloc_device<Camera>(1, q);
    MeshObject* dev_meshObjects =
        sycl::malloc_device<MeshObject>(meshObjectCount, q);

    Tri* dev_tris = sycl::malloc_device<Tri>(triCount, q);
    BVH* dev_bvh = sycl::malloc_device<BVH>(1, q);
    int* dev_triIndices = sycl::malloc_device<int>(triCount, q);

    float* dev_passes = sycl::malloc_device<float>(PASSES_COUNT * 1920 * 1080 * 4, q);
    unsigned int* dev_samples = sycl::malloc_device<unsigned int>(1920 * 1080, q);
    RngGenerator* dev_randstate = sycl::malloc_device<RngGenerator>(1920 * 1080, q);

    geometryMemory += sizeof(MeshObject) * meshObjectCount +
                      sizeof(Tri) * triCount + sizeof(BVH) +
                      sizeof(int) * triCount + sizeof(BVH);


    printf("Copying to GPU");

    q.memcpy(&(dev_scene->meshObjectCount), &meshObjectCount,
             sizeof(unsigned int))
        .wait();
    q.memcpy(&(dev_scene->triCount), &triCount, sizeof(unsigned int)).wait();

    q.memcpy(dev_meshObjects, meshObjects, sizeof(MeshObject) * meshObjectCount)
        .wait();
    q.memcpy(dev_camera, camera, sizeof(Camera)).wait();
    q.memcpy(dev_tris, tris, sizeof(Tri) * triCount).wait();
    q.memcpy(dev_bvh, bvh, sizeof(BVH)).wait();
    q.memcpy(dev_triIndices, bvh->triIndices, sizeof(int) * triCount).wait();

    for (int i = 0; i < meshObjectCount; i++) {
        q.memcpy(&(dev_meshObjects[i].tris), &dev_tris, sizeof(Tri*)).wait();
    }

    printf("Pointer binding...\n");

    q.memcpy(&(dev_scene->meshObjects), &(dev_meshObjects), sizeof(MeshObject*))
        .wait();
    q.memcpy(&(dev_scene->camera), &(dev_camera), sizeof(Camera*)).wait();
    q.memcpy(&(dev_scene->tris), &(dev_tris), sizeof(Tri*)).wait();
    q.memcpy(&(dev_scene->bvh), &(dev_bvh), sizeof(BVH*)).wait();
    q.memcpy(&(dev_scene->dev_passes), &(dev_passes), sizeof(float*)).wait();
    q.memcpy(&(dev_scene->dev_samples), &(dev_samples), sizeof(unsigned int*)).wait();
    q.memcpy(&(dev_scene->dev_randstate), &(dev_randstate), sizeof(RngGenerator*)).wait();

    q.memcpy(&(dev_bvh->tris), &(dev_tris), sizeof(Tri*)).wait();
    q.memcpy(&(dev_bvh->triIndices), &(dev_triIndices), sizeof(int*)).wait();

    // POINTLIGHT SETUP

    unsigned int pointLightCount = scene->pointLightCount();

    PointLight* dev_pointLights =
        sycl::malloc_device<PointLight>(pointLightCount, q);

    q.memcpy(&dev_scene->pointLightCount, &pointLightCount,
             sizeof(unsigned int))
        .wait();

    q.memcpy(dev_pointLights, scene->getPointLights(),
             sizeof(PointLight) * pointLightCount)
        .wait();
    q.memcpy(&(dev_scene->pointLights), &(dev_pointLights), sizeof(PointLight*))
        .wait();

    // MATERIAL SETUP

    unsigned int materialCount = scene->materialCount();

    q.memcpy(&dev_scene->materialCount, &materialCount, sizeof(unsigned int))
        .wait();

    Material* dev_materials = sycl::malloc_device<Material>(materialCount, q);

    q.memcpy(dev_materials, scene->getMaterials(),
             sizeof(Material) * materialCount)
        .wait();

    q.memcpy(&(dev_scene->materials), &(dev_materials), sizeof(Material*))
        .wait();

    // TEXTURES

    unsigned int textureCount = scene->textureCount();

    Texture* textures = scene->getTextures();

    q.memcpy(&dev_scene->textureCount, &textureCount, sizeof(unsigned int))
        .wait();

    Texture* dev_textures = sycl::malloc_device<Texture>(textureCount, q);

    textureMemory += sizeof(Texture) * textureCount;

    q.memcpy(dev_textures, textures, sizeof(Texture) * textureCount).wait();

    for (int i = 0; i < textureCount; i++) {
        float* textureData = sycl::malloc_device<float>(
            textures[i].width * textures[i].height * 3, q);
        textureMemory +=
            sizeof(float) * textures[i].width * textures[i].height * 3;

        q.memcpy(textureData, textures[i].data,
                 sizeof(float) * textures[i].width * textures[i].height * 3)
            .wait();
        q.memcpy(&(dev_textures[i].data), &textureData, sizeof(float*)).wait();

        printf("Texture %ld copied, %dpx x %ldpx\n", i, textures[i].width,
               textures[i].height);
    }

    q.memcpy(&(dev_scene->textures), &(dev_textures), sizeof(Texture*)).wait();

    // HDRI

    HDRI* hdri = &scene->hdri;

    HDRI* dev_hdri = sycl::malloc_device<HDRI>(1, q);

    float* dev_data = sycl::malloc_device<float>(
        hdri->texture.height * hdri->texture.width * 3, q);
    float* dev_cdf = sycl::malloc_device<float>(
        hdri->texture.height * hdri->texture.width, q);

    textureMemory += sizeof(HDRI) + sizeof(float) * hdri->texture.height *
                                        hdri->texture.width * 4;

    q.memcpy(dev_hdri, hdri, sizeof(HDRI)).wait();
    q.memcpy(dev_data, hdri->texture.data,
             sizeof(float) * hdri->texture.height * hdri->texture.width * 3)
        .wait();
    q.memcpy(dev_cdf, hdri->cdf,
             sizeof(float) * hdri->texture.height * hdri->texture.width)
        .wait();

    q.memcpy((&dev_hdri->texture.data), &(dev_data), sizeof(float*)).wait();
    q.memcpy(&(dev_hdri->cdf), &(dev_cdf), sizeof(float*)).wait();
    q.memcpy(&(dev_scene->hdri), &(dev_hdri), sizeof(float*)).wait();

    printf("%luMB of geometry data copied\n",
           (geometryMemory / (1024L * 1024L)));

    auto cg = [&](sycl::handler& h) {
        sycl::stream out = sycl::stream(1024, 256, h);
       
        h.parallel_for(sycl::range(camera->xRes * camera->yRes),
                       [=](sycl::id<1> i) {
                           setupKernel(dev_scene, i, out);
                       });
    };

    printf("Submiting to the queue...");

    q.submit(cg).wait();

    for (int i = 0; i < 10; i++) {
   
        auto rg = [&](sycl::handler& h) {

            sycl::stream out = sycl::stream(1024, 256, h);

            h.parallel_for(
                sycl::range(camera->xRes * camera->yRes), [=](sycl::id<1> i) {
                    renderingKernel(dev_scene, i, out);
                });
        };

        printf("Submiting to the queue...");

        // 
        q.submit(rg);
    }

    return 0;
}

int renderCuda(sycl::queue& q, Scene* scene, int sampleTarget) {
    /*

    for (int i = 0; i < sampleTarget; i++) {
        auto cg = [&](sycl::handler& h) {
            sycl::stream out = sycl::stream(1024, 256, h);

            h.parallel_for(
                sycl::range(scene->camera.xRes * scene->camera.yRes),
                [=](sycl::id<1> i) { renderingKernel(nullptr, i, out); });
        };

        printf("Submiting to the queue...");

        q.submit(cg).wait();
    }
    */
    return 0;
}

int getBuffers(dev_Scene* dev_scene, sycl::queue& q, RenderData& renderData, int* pathcountBuffer,
               int size) {


    float* a = new float[1920 * 1080 * 4 * PASSES_COUNT];

    q.memcpy(a, dev_scene->dev_passes, 1920 * 1080 * 4 * PASSES_COUNT).wait();

    for (int i = 0; i < PASSES_COUNT; i++) {
        printf("\nRetrieving pass %d\n", i);
        for (int j = 0; j < size; j++) {
            int n = i * size * 4;
            renderData.passes[i][j * 4 + 0] = a[n + j * 4 + 0];
            renderData.passes[i][j * 4 + 1] = a[n + j * 4 + 1];
            renderData.passes[i][j * 4 + 2] = a[n + j * 4 + 2];
            renderData.passes[i][j * 4 + 3] = a[n + j * 4 + 3];
        }
    }
    
    delete[] a;

    return 0;
}

int getSamples(dev_Scene* dev_scene, sycl::queue& q) {

    unsigned int i;
    unsigned int* a = new unsigned int[1920 * 1080];
    q.memcpy(a, dev_scene->dev_samples, 1920 * 1080).wait();
    i = a[0];
    delete[] a;
    return i;
}

void printHDRISampling(HDRI hdri, int samples) {
    for (int i = 0; i < samples; i++) {
        float r = ((float)i) / ((float)samples);

        Vector3 sample = hdri.sample(r);

        printf("%d, %d,", (int)sample.x, (int)sample.y);
    }

    float sum = 0;

    for (int i = 0; i < 1024; i++) {
        for (int j = 0; j < 2048; j++) {
            sum += hdri.pdf(j, i);
        }
    }

    printf("PDF TOTAL %f", sum);
}

void printBRDFMaterial(Material material, int samples) {
    HitData hitdata;

    hitdata.albedo = material.albedo;
    hitdata.emission = material.emission;
    hitdata.roughness = material.roughness;
    hitdata.metallic = material.metallic;
    hitdata.clearcoatGloss = material.clearcoatGloss;
    hitdata.clearcoat = material.clearcoat;
    hitdata.anisotropic = material.anisotropic;
    hitdata.eta = material.eta;
    hitdata.transmission = material.transmission;
    hitdata.specular = material.specular;
    hitdata.specularTint = material.specularTint;
    hitdata.sheenTint = material.sheenTint;
    hitdata.subsurface = material.subsurface;
    hitdata.sheen = material.sheen;

    createBasis(hitdata.normal, hitdata.tangent, hitdata.bitangent);

    Vector3 inLight = Vector3(1, -1, 0).normalized();

    for (int i = 0; i < sqrt(samples); i++) {
        for (int j = 0; j < sqrt(samples); j++) {
            float cosPhi = 2.0f * ((float)i / (float)sqrt(samples)) - 1.0f;
            float sinPhi = std::sqrt(1.0f - cosPhi * cosPhi);
            float theta = 2 * PI * ((float)j / (float)sqrt(samples));

            float x = sinPhi * std::sinf(theta);
            float y = cosPhi;
            float z = sinPhi * std::cosf(theta);

            Vector3 rndVector = Vector3(x, y, z).normalized();

            Ray r = Ray(Vector3(0), inLight);

            float brdf = DisneyEval(r, hitdata, rndVector).length();

            printf("%f,%f,%f,%f;", rndVector.x, rndVector.y, rndVector.z, brdf);
        }
    }
}
