﻿#include "kernel.h"

#include <math.h>
#include <stdio.h>

#include "sycl.h"
#include <iostream>
#include <limits>

#include "BVH.h"
#include "Camera.h"
#include "Definitions.h"
#include "Disney.h"
#include "HDRI.h"
#include "Hit.h"
#include "Material.h"
#include "Math.hpp"
#include "PointLight.h"
#include "SYCLCopy.h"
#include "shader.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

uint32_t jenkins_one_at_a_time_hash(uint32_t seed) {
    uint32_t hash = 0;
    for (size_t i = 0; i < sizeof(uint32_t); i++) {
        hash += (seed >> (i * 8)) & 0xFF;
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}

RngGenerator::RngGenerator(uint32_t _seed) {
    this->state = jenkins_one_at_a_time_hash(_seed + 1);
}

float RngGenerator::next() {
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return (static_cast<float>(state) / static_cast<float>(UINT_MAX));
}

//TODO: move this function
Passes parsePass(std::string s_pass) {

    std::transform(s_pass.begin(), s_pass.end(), s_pass.begin(), ::tolower);

    Passes pass = BEAUTY;

    if (s_pass.compare("beauty") == 0) {
        pass = BEAUTY;
    }
    else if (s_pass.compare("denoise") == 0) {
        pass = DENOISE;
    }
    else if (s_pass.compare("normal") == 0) {
        pass = NORMAL;
    }
    else if (s_pass.compare("tangent") == 0) {
        pass = TANGENT;
    }
    else if (s_pass.compare("bitangent") == 0) {
        pass = BITANGENT;
    }

    return pass;
}

// After hitting some surface, generate HitData which contains the information of such a hit.
void generateHitData(dev_Scene* dev_scene_g, Material* material,
    HitData& hitdata, Hit hit) {
    Vector3 tangent, bitangent, normal;

    normal = hit.normal;
    tangent = hit.tangent;
    bitangent = hit.bitangent;

    if (material->albedoTextureID < 0) {
        hitdata.albedo = material->albedo;
    }
    else {
        hitdata.albedo = dev_scene_g->textures[material->albedoTextureID]
            .getValueFromUVFiltered(hit.tu, hit.tv);
    }

    if (material->opacityTextureID < 0) {
        hitdata.opacity = material->opacity;
    }
    else {
        hitdata.opacity = dev_scene_g->textures[material->opacityTextureID]
            .getValueFromUVFiltered(hit.tu, hit.tv).x;
    }

    if (material->emissionTextureID < 0) {
        hitdata.emission = material->emission;
    }
    else {
        hitdata.emission = dev_scene_g->textures[material->emissionTextureID]
            .getValueFromUVFiltered(hit.tu, hit.tv);
    }

    if (material->roughnessTextureID < 0) {
        hitdata.roughness = material->roughness;
    }
    else {
        hitdata.roughness = dev_scene_g->textures[material->roughnessTextureID]
            .getValueFromUVFiltered(hit.tu, hit.tv)
            .x;
    }

    if (material->metallicTextureID < 0) {
        hitdata.metallic = material->metallic;
    }
    else {
        hitdata.metallic = dev_scene_g->textures[material->metallicTextureID]
            .getValueFromUVFiltered(hit.tu, hit.tv)
            .x;
    }

    if (material->transmissionTextureID < 0) {
        hitdata.transmission = material->transmission;
    }
    else {
        hitdata.transmission = dev_scene_g->textures[material->transmissionTextureID]
            .getValueFromUVFiltered(hit.tu, hit.tv)
            .x;
    }

    if (material->normalTextureID < 0) {
        hitdata.normal = normal;
    }
    else {
        Vector3 ncolor =
            dev_scene_g->textures[material->normalTextureID].getValueFromUV(
                hit.tu, hit.tv);
        Vector3 localNormal = (ncolor * 2) - Vector3::One();
        Vector3 worldNormal =
            (localNormal.x * tangent - localNormal.y * bitangent +
                localNormal.z * hit.normal)
            .normalized();

        hitdata.normal = worldNormal;
    }

    //TODO: Figure out why it's necessary to gamma correct this values
    hitdata.roughness = sycl::pow(hitdata.roughness,2.2f);
    hitdata.metallic = sycl::pow(hitdata.metallic, 2.2f);

    hitdata.clearcoatGloss = material->clearcoatGloss;
    hitdata.clearcoat = material->clearcoat;
    hitdata.anisotropic = material->anisotropic;
    hitdata.eta = material->eta;
    hitdata.specular = material->specular;
    hitdata.specularTint = material->specularTint;
    hitdata.sheenTint = material->sheenTint;
    hitdata.subsurface = material->subsurface;
    hitdata.sheen = material->sheen;
    hitdata.ax = material->ax;
    hitdata.ay = material->ay;

    hitdata.gnormal = hit.gnormal;
    hitdata.tangent = tangent;
    hitdata.bitangent = bitangent;
    hitdata.position = hit.position;
    hitdata.triIdx = hit.triIdx;
}


// TODO: move this to outside a kernel. Reseting buffers and initializations can be done with memset.
void setupKernel(dev_Scene* dev_scene_g, int idx) {

    if (idx >= dev_scene_g->x_res * dev_scene_g->y_res) {
        return;
    }

    //// Initializing RNG
    dev_scene_g->dev_randstate[idx] = RngGenerator(idx);
    
    
    // Setting pixel buffers to zero
    for (int i = 0; i < PASSES_COUNT; i++) {
        dev_scene_g->dev_passes[(i * dev_scene_g->x_res * dev_scene_g->y_res *
            4) +
            (4 * idx + 0)] = 0.0f;
        dev_scene_g->dev_passes[(i * dev_scene_g->x_res * dev_scene_g->y_res *
            4) +
            (4 * idx + 1)] = 0.0f;
        dev_scene_g->dev_passes[(i * dev_scene_g->x_res * dev_scene_g->y_res *
            4) +
            (4 * idx + 2)] = 0.0f;
        dev_scene_g->dev_passes[(i * dev_scene_g->x_res * dev_scene_g->y_res *
            4) +
            (4 * idx + 3)] = 1.0f;
    }

    // Reset the amount of samples for each pixel. (1.1s)
    dev_scene_g->dev_samples[idx] = 1;

    // (0s)
    if (idx == 0) {
        int triSum = 0;
        for (int i = 0; i < dev_scene_g->meshObjectCount; i++) {
            dev_scene_g->meshObjects[i].tris += triSum;
            triSum += dev_scene_g->meshObjects[i].triCount;
        }
    }
}



// Throw a ray to the scene, and calculate the hit point.
Hit throwRay(Ray ray, dev_Scene* scene) {
    Hit nearestHit = Hit();

#if USEBVH
    scene->bvh->transverse(ray, nearestHit);
#else
    for (int j = 0; j < scene->meshObjectCount; j++) {
        Hit hit = Hit();

        if (scene->meshObjects[j].hit(ray, hit)) {
            if (!nearestHit.valid) {
                nearestHit = hit;
            }

            if ((hit.position - ray.origin).length() <
                (nearestHit.position - ray.origin).length()) {
                nearestHit = hit;
            }
        }
    }
#endif
    return nearestHit;
}

// Copy constructor for dev_scene. 
// TODO: redo this.
dev_Scene::dev_Scene(Scene* scene) {

    camera = &(scene->camera);

    meshObjectCount = scene->meshObjectCount();
    materialCount = scene->materialCount();
    textureCount = scene->textureCount();
    triCount = scene->triCount();
    pointLightCount = scene->pointLightCount();

    pointLights = scene->getPointLights();
    meshObjects = scene->getMeshObjects();
    materials = scene->getMaterials();
    textures = scene->getTextures();
    hdri = &scene->hdri;

    tris = scene->getTris();
    bvh = scene->buildBVH();

    x_res = scene->x_res;
    y_res = scene->y_res;

 }

// Calculate the light amount from all the light-points.
Vector3 pointLight(Ray ray, HitData hitdata, dev_Scene* scene, Vector3 point,
    float& pdf, float r1) {
    if (scene->pointLightCount <= 0) {
        pdf = 0;
        return Vector3::Zero();
    }

    pdf = (static_cast<float>(scene->pointLightCount)) / (2.0f * PIF);

    // Retrieve a random light
    PointLight light = scene->pointLights[static_cast<int>(scene->pointLightCount * r1)];

    Vector3 newDir = (light.position - point).normalized();

    float dist = (light.position - point).length();

    // Test if the point is visible from the light
    Ray shadowRay(point + newDir * 0.001f, newDir);
    Hit shadowHit = throwRay(shadowRay, scene);
    float shadowDist = (shadowHit.position - point).length();

    if (shadowHit.valid && shadowDist < dist) {
        return Vector3();
    }

    // Quadratic attenuation
    Vector3 pointLightValue = (light.radiance / (dist * dist));

    //Vector3 brdfDisney = DisneyEval(ray, hitdata, newDir);

    //return pointLightValue * brdfDisney *
    //       abs(Vector3::dot(newDir, hitdata.normal)) / pdf;
}

// Sampling HDRI
// The main idea is to get a random point of the HDRI, weighted by their
// importance and then get the direction from the center to that point as if
// that pixel would be in a sphere of infinite radius.
Vector3 hdriLight(Ray ray, dev_Scene* scene, Vector3 point, HitData hitdata, RngGenerator& rnd, float& pdf) {

    /*

    if (!HDRIIS) {

        Vector3 newDir = uniformSampleSphere(rnd.next(), rnd.next()).normalized();

        if (Vector3::dot(newDir, hitdata.gnormal) < 0)
            newDir *= -1;

        float u, v;

        Texture::sphericalMapping(Vector3(), -1 * newDir, 1, u, v);

        Ray shadowRay(hitdata.position + hitdata.gnormal * 0.001, newDir);

        Hit shadowHit = throwRay(shadowRay, scene, -2);

        if (shadowHit.valid && shadowHit.triIdx != hitdata.triIdx)
            return Vector3();

        Vector3 hdriValue = scene->hdri->texture.getValueFromUV(u, v);
        Vector3 disneyBrdf = DisneyEval(-newDir, hitdata, -ray.direction);

        //pdf = DisneyPdf(ray, hitdata, newDir);
        pdf = 1 / (PI * 4);

        return hdriValue * disneyBrdf;

    }
    else {

        Vector3 textCoordinate = scene->hdri->sample(rnd.next());

        float nu = textCoordinate.x / (float)scene->hdri->texture.width;
        float nv = textCoordinate.y / (float)scene->hdri->texture.height;

        float iu = scene->hdri->texture.inverseTransformUV(nu, nv).x;
        float iv = scene->hdri->texture.inverseTransformUV(nu, nv).y;

        Vector3 newDir =
            -scene->hdri->texture.reverseSphericalMapping(iu, iv).normalized();

        Ray shadowRay(point + newDir * 0.0001, newDir);

        Hit shadowHit = throwRay(shadowRay, scene, -1);

        if (shadowHit.valid) return Vector3();

        Vector3 hdriValue = scene->hdri->texture.getValueFromUV(iu, iv);

        Vector3 brdfDisney = DisneyEval(ray.direction, hitdata, newDir);

        pdf = scene->hdri->pdf(iu * scene->hdri->texture.width,
            iv * scene->hdri->texture.height);

        return brdfDisney * abs(Vector3::dot(newDir, hitdata.normal)) *
            (hdriValue / pdf);
    }*/
}

// Calculate the initial camera ray. 
// TODO: clean up the rotation mess
void calculateCameraRay(int x, int y, dev_Scene& scene, Camera& camera, Ray& ray, float r1,
    float r2, float r3, float r4, float r5) {
    // Relative coordinates for the point where the first ray will be launched
    float dx = camera.position.x +
        (static_cast<float>(x)) / (static_cast<float>(scene.x_res)) * camera.sensorWidth;
    float dy = camera.position.y +
        (static_cast<float>(y)) / (static_cast<float>(scene.y_res)) * camera.sensorHeight;

    // Absolute coordinates for the point where the first ray will be launched
    float odx = (-camera.sensorWidth / 2.0f) + dx;
    float ody = (-camera.sensorHeight / 2.0f) + dy;

    // Random part of the sampling offset so we get antialasing
    float rx = (1.0f / static_cast<float>(scene.x_res)) * (r1 - 0.5f) * camera.sensorWidth;
    float ry = (1.0f / static_cast<float>(scene.y_res)) * (r2 - 0.5f) * camera.sensorHeight;

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

    rotation *= (PIF / 180.0f);

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

        rIPx *= diameter * 0.5f;
        rIPy *= diameter * 0.5f;

        Vector3 rIP(rIPx, rIPy, 0);

        Vector3 dirXRot = Vector3(
            rIP.x, rIP.y * sycl::cos(rotation.x) - rIP.z * sycl::sin(rotation.x),
            rIP.y * sycl::sin(rotation.x) + rIP.z * sycl::cos(rotation.x));
        Vector3 dirYRot = Vector3(
            dirXRot.x * sycl::cos(rotation.y) + dirXRot.z * sycl::sin(rotation.y),
            dirXRot.y,
            dirXRot.z * sycl::cos(rotation.y) - dirXRot.x * sycl::sin(rotation.y));
        Vector3 dirZRot = Vector3(
            dirYRot.x * sycl::cos(rotation.z) - dirYRot.y * sycl::sin(rotation.z),
            dirYRot.x * sycl::sin(rotation.z) + dirYRot.y * sycl::cos(rotation.z),
            dirYRot.z);

        Vector3 orig = camera.position + dirZRot;

        // Blurred ray
        ray = Ray(orig, focusPoint - orig);
    }
}


// Main Kernel
void renderingKernel(dev_Scene* scene, int idx, int samples) {

    if (idx >= scene->x_res * scene->y_res) {
        return;
    }

    RngGenerator rnd = scene->dev_randstate[idx];

    unsigned int sa = scene->dev_samples[idx];

    Ray ray;

    int x = (idx % scene->x_res);
    int y = (idx / scene->x_res);

    calculateCameraRay(x, y, *scene, *scene->camera, ray, rnd.next(), rnd.next(),
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

    for (i = 0; i < 5; i++) {

        HitData hitdata;

        int materialID = 0;

        Hit nearestHit = throwRay(ray, scene);

        // If the ray goes outside the world, sample the environment.
        if (!nearestHit.valid) {
            float u, v;
            Texture::sphericalMapping(Vector3(), -1 * ray.direction, 1, u, v);
            light += reduction * scene->hdri->texture.getValueFromUVFiltered(u, v);
            break;
        }

        materialID = nearestHit.material_id;

        Material* material = &scene->materials[materialID];

        generateHitData(scene, material, hitdata, nearestHit);

        if (material->albedoShaderID != -1) {
            hitdata.albedo.x = 0;
            hitdata.albedo.y = 0;
            hitdata.albedo.z = 0;
            //asl_shade0_(nearestHit.position.x, nearestHit.position.y, nearestHit.position.z, ray.direction.x, ray.direction.y, ray.direction.z, hitdata.normal.x, hitdata.normal.y, hitdata.normal.z, hitdata.gnormal.x, hitdata.gnormal.y, hitdata.gnormal.z, nearestHit.tu, nearestHit.tv, hitdata.albedo.x, hitdata.albedo.y, hitdata.albedo.z);
            asl_shade(material->albedoShaderID, nearestHit.position.x, nearestHit.position.y, nearestHit.position.z, ray.direction.x, ray.direction.y, ray.direction.z, hitdata.normal.x, hitdata.normal.y, hitdata.normal.z, hitdata.gnormal.x, hitdata.gnormal.y, hitdata.gnormal.z, nearestHit.tu, nearestHit.tv, hitdata.albedo.x, hitdata.albedo.y, hitdata.albedo.z);
        }

        
        if (rnd.next() <= hitdata.opacity) {

            Vector3 wo = -ray.direction;     

            Vector3 textCoordinate = scene->hdri->sample(rnd.next());

            Vector3 wibrdf = DisneySample(hitdata, wo, hitdata.normal, rnd.next(), rnd.next(), rnd.next());

            float nu = textCoordinate.x / static_cast<float>(scene->hdri->texture.width);
            float nv = textCoordinate.y / static_cast<float>(scene->hdri->texture.height);

            float iu = scene->hdri->texture.inverseTransformUV(nu, nv).x;
            float iv = scene->hdri->texture.inverseTransformUV(nu, nv).y;

            Vector3 wihdri = -scene->hdri->texture.reverseSphericalMapping(iu, iv).normalized();

            Ray shadowRay(hitdata.position + hitdata.normal * 0.001f, wihdri);
            Hit shadowHit = throwRay(shadowRay, scene);

            Vector3 hdriValue = scene->hdri->texture.getValueFromUV(iu, iv);

            if (shadowHit.valid && shadowHit.triIdx != hitdata.triIdx) {
                hdriValue = Vector3();
            }
                

            float hdripdf = scene->hdri->pdf(iu * scene->hdri->texture.width, iv * scene->hdri->texture.height);

            Vector3 hdriInt = hdriValue * DisneyEval(hitdata, wo, hitdata.normal, wihdri) * sycl::abs(Vector3::dot(wihdri, hitdata.normal)) / hdripdf;
            
            float brdfpdf = DisneyPdf(hitdata, wo, hitdata.normal, wibrdf);

            float hw = hdripdf / (brdfpdf + hdripdf);
            float bw = brdfpdf / (brdfpdf + hdripdf);

            //Vector3 hdriInt = textCoordinate;
            light += reduction * (hitdata.emission + hdriInt);

            reduction *= DisneyEval(hitdata, wo, hitdata.normal, wibrdf) * sycl::abs(Vector3::dot(wibrdf, hitdata.normal)) / brdfpdf;

        
            // First hit
            if (i == 0) {
                normal = hitdata.normal;
                tangent = hitdata.tangent;
                bitangent = hitdata.bitangent;
            }

            ray = Ray(nearestHit.position + wibrdf * 0.001f, wibrdf);
        }
        else {
            ray = Ray(nearestHit.position + ray.direction * 0.001f, ray.direction);
            
        }
    }

    // TODO: parametrize light 

    light = clamp(light, 0, 10);

    // TODO: cleanup
    if (!sycl::isnan(light.x) && !sycl::isnan(light.y) &&
        !sycl::isnan(light.z)) {
        if (sa > 0) {
            for (int pass = 0; pass < PASSES_COUNT; pass++) {
                if (pass != DENOISE) {
                    scene->dev_passes[(pass * scene->x_res * scene->y_res * 4) + (4 * idx + 0)] *=
                        (static_cast<float>(sa)) / (static_cast<float>((sa + 1)));
                    scene->dev_passes[(pass * scene->x_res * scene->y_res * 4) + (4 * idx + 1)] *=
                        (static_cast<float>(sa)) / (static_cast<float>((sa + 1)));
                    scene->dev_passes[(pass * scene->x_res * scene->y_res * 4) + (4 * idx + 2)] *=
                        (static_cast<float>(sa)) / (static_cast<float>((sa + 1)));
                }
            }
        }

        scene->dev_passes[(BEAUTY * scene->x_res * scene->y_res * 4) + (4 * idx + 0)] +=
            light.x / (static_cast<float>(sa + 1));
        scene->dev_passes[(BEAUTY * scene->x_res * scene->y_res * 4) + (4 * idx + 1)] +=
            light.y / (static_cast<float>(sa + 1));
        scene->dev_passes[(BEAUTY * scene->x_res * scene->y_res * 4) + (4 * idx + 2)] +=
            light.z / (static_cast<float>(sa + 1));

        scene->dev_passes[(NORMAL * scene->x_res * scene->y_res * 4) + (4 * idx + 0)] +=
            normal.x / (static_cast<float>(sa + 1));
        scene->dev_passes[(NORMAL * scene->x_res * scene->y_res * 4) + (4 * idx + 1)] +=
            normal.y / (static_cast<float>(sa + 1));
        scene->dev_passes[(NORMAL * scene->x_res * scene->y_res * 4) + (4 * idx + 2)] +=
            normal.z / (static_cast<float>(sa + 1));

        scene->dev_passes[(TANGENT * scene->x_res * scene->y_res * 4) + (4 * idx + 0)] +=
            tangent.x / (static_cast<float>(sa + 1));
        scene->dev_passes[(TANGENT * scene->x_res * scene->y_res * 4) + (4 * idx + 1)] +=
            tangent.y / (static_cast<float>(sa + 1));
        scene->dev_passes[(TANGENT * scene->x_res * scene->y_res * 4) + (4 * idx + 2)] +=
            tangent.z / (static_cast<float>(sa + 1));

        scene->dev_passes[(BITANGENT * scene->x_res * scene->y_res * 4) + (4 * idx + 0)] +=
            bitangent.x / (static_cast<float>(sa + 1));
        scene->dev_passes[(BITANGENT * scene->x_res * scene->y_res * 4) + (4 * idx + 1)] +=
            bitangent.y / (static_cast<float>(sa + 1));
        scene->dev_passes[(BITANGENT * scene->x_res * scene->y_res * 4) + (4 * idx + 2)] +=
            bitangent.z / (static_cast<float>(sa + 1));

        scene->dev_samples[idx]++;
    }
    scene->dev_randstate[idx] = rnd;
}


//TODO: refactor this code.

void renderSetup(sycl::queue& q, Scene* scene, dev_Scene* dev_scene, unsigned int target_samples, unsigned int block_size) {

    LOG(info) << "Initializing rendering at " << target_samples << " sample target and blocksize: " << block_size;

    dev_Scene* temp = new dev_Scene(scene);

    copy_scene(temp, dev_scene, q);

    sycl::range global{ (scene->x_res / block_size) * block_size, (scene->y_res / block_size) * block_size };
    sycl::range local{ block_size,block_size };

    LOG(debug) << "Starting setup kernels";

    try {
        q.submit([&](cl::sycl::handler& h) {

            h.parallel_for(sycl::nd_range{ global, local },
            [=](sycl::nd_item<2> it) {
                    setupKernel(dev_scene, it.get_global_id(0) * dev_scene->y_res + it.get_global_id(1));
                });
            }).wait();
    } catch (std::exception const& e) {
        LOG(error) << "Error setting up kernels: " << e.what();
    }

    LOG(info) << "Setup finished";
}


void kernel_render_enqueue(sycl::queue& q, int target_samples, unsigned long long block_size, Scene* scene, dev_Scene* dev_scene) {

    LOG(trace) << "kernel_render_enqueue::start";
    
    try {

        sycl::range global{ scene->x_res + (block_size - (scene->x_res % block_size)),scene->y_res + (block_size - (scene->y_res % block_size)) };
        sycl::range local{ block_size,block_size };

        for (int i = 0; i < target_samples; i++) {

            LOG(trace) << "submitting one sample";

            q.submit([&](cl::sycl::handler& h) {

                h.parallel_for<KernelNameTest>(sycl::nd_range{ global, local },
                [=](sycl::nd_item<2> it) {
                        renderingKernel(dev_scene, it.get_global_id(0) * dev_scene->y_res + it.get_global_id(1), target_samples);
                    });
                }).wait();
        }
    }
    catch (std::exception const& e) {
        LOG(error) << "Error adding rendering kernels to the queue: " << e.what();
    }
    LOG(info) << "All samples added to the queue";
}

bool is_compatible(sycl::device& device) {
    bool compatible = false;
    try {
        // Any call to KernelName needs to be in the same compiler unit as the parallel_for call where is called.
        compatible = sycl::is_compatible<KernelNameTest>(device);
    }
    catch (std::exception const& e) {
        LOG(info) << "ERROR: " << e.what();
    }

    LOG(info) << "COMPATIBLE: " << compatible;
    return compatible;
}