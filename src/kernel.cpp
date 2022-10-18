#include "kernel.h"

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
#include "OslMaterial.hpp"
#include "lan/calc.tab.hpp"
#include "SYCLCopy.h"

#define STB_IMAGE_IMPLEMENTATION
#include "libs/stb_image.h"


RngGenerator::RngGenerator(uint32_t _seed) {
    this->state = _seed + 1;

    for (int i = 0; i < _seed; i++)
        this->next();
}

float RngGenerator::next() {
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return ((float)state / (float)UINT_MAX);
}

//TODO move this function
Passes parsePass(std::string s_pass) {

    std::transform(s_pass.begin(), s_pass.end(), s_pass.begin(), ::tolower);

    Passes pass = BEAUTY;

    if (s_pass.compare("beauty") == 0)
        pass = BEAUTY;
    if (s_pass.compare("denoise") == 0)
        pass = DENOISE;
    if (s_pass.compare("normal") == 0)
        pass = NORMAL;
    if (s_pass.compare("tangent") == 0)
        pass = TANGENT;
    if (s_pass.compare("bitangent") == 0)
        pass = BITANGENT;

    return pass;
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

    if (material->normalTextureID < 0) {
        hitdata.normal = normal;
    }
    else {
        Vector3 ncolor =
            dev_scene_g->textures[material->normalTextureID].getValueFromUV(
                hit.tu, hit.tv);
        Vector3 localNormal = (ncolor * 2) - 1;
        Vector3 worldNormal =
            (localNormal.x * tangent - localNormal.y * bitangent +
                localNormal.z * hit.normal)
            .normalized();

        hitdata.normal = worldNormal;
    }


    //hitdata.roughness = sycl::pow(hitdata.roughness,2.2f);
    //hitdata.metallic = sycl::pow(hitdata.metallic, 2.2f);

    hitdata.roughness = hitdata.roughness;
    hitdata.metallic = hitdata.metallic;

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
    hitdata.ax = material->ax;
    hitdata.ay = material->ay;

    hitdata.gnormal = hit.gnormal;
    hitdata.tangent = tangent;
    hitdata.bitangent = bitangent;
    hitdata.position = hit.position;
    hitdata.triIdx = hit.triIdx;
}


void setupKernel(dev_Scene* dev_scene_g, int idx, sycl::stream out, OslMaterial* dev_osl) {

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

        out << "PRINTING EMPTY\n";

        dev_osl->program->print(out);

        out << "COMPUTING\n";

        dev_osl->compute_ci(out);

        out << "PRINTING FULL\n";

        //dev_osl->program->print(out);

        dev_osl->vars.print(out);

        int triSum = 0;
        for (int i = 0; i < dev_scene_g->meshObjectCount; i++) {
            dev_scene_g->meshObjects[i].tris += triSum;
            triSum += dev_scene_g->meshObjects[i].triCount;
        }
    }
}

Hit throwRay(Ray ray, dev_Scene* scene, int ignoreID) {
    Hit nearestHit = Hit();

#if USEBVH
    scene->bvh->transverse(ray, nearestHit, ignoreID);
#else
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

    //float* dev_passes;
    //unsigned int* dev_samples;
    //RngGenerator* dev_randstate;
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
    Hit shadowHit = throwRay(shadowRay, scene, -1);
    float shadowDist = (shadowHit.position - point).length();

    if (shadowHit.valid && shadowDist < dist) return Vector3();

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



void renderingKernel(dev_Scene* scene, int idx, int s) {

    if (idx >= scene->camera->xRes * scene->camera->yRes)
        return;

    RngGenerator rnd = scene->dev_randstate[idx];

    unsigned int sa = scene->dev_samples[idx];

    Ray ray;

    int x = (idx % scene->camera->xRes);
    int y = (idx / scene->camera->xRes);


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

    for (i = 0; i < 5; i++) {

        HitData hitdata;

        int materialID = 0;

        Hit nearestHit = throwRay(ray, scene, -1);

        if (!nearestHit.valid) {
            float u, v;
            Texture::sphericalMapping(Vector3(), -1 * ray.direction, 1, u, v);
            light += reduction * scene->hdri->texture.getValueFromUVFiltered(u, v);
            break;
        }


        materialID = scene->meshObjects[nearestHit.objectID].materialID;

        Material* material = &scene->materials[materialID];

        generateHitData(scene, material, hitdata, nearestHit);

        if (rnd.next() <= hitdata.opacity) {

            Vector3 wo = -ray.direction;     

           // Vector3 wihdri = uniformSampleSphere(rnd.next(), rnd.next()).normalized();
            Vector3 wibrdf = DisneySample(hitdata, wo, hitdata.normal, rnd.next(), rnd.next(), rnd.next());

            //if (Vector3::dot(wihdri, hitdata.gnormal) < 0)
            //    wihdri *= -1;

            //float u, v;

            //Texture::sphericalMapping(Vector3(), -1 * wihdri, 1, u, v);

            //Ray shadowRay(hitdata.position + hitdata.gnormal * 0.001, wihdri);
            //Hit shadowHit = throwRay(shadowRay, scene, -2);

            //Vector3 hdriValue = scene->hdri->texture.getValueFromUV(u, v);

            //if (shadowHit.valid && shadowHit.triIdx != hitdata.triIdx)
            //    hdriValue = Vector3();

            //Vector3 hdriInt = hdriValue * DisneyEval(hitdata, wo, hitdata.gnormal, wihdri) * abs(Vector3::dot(wihdri, hitdata.gnormal)) / (1 / (1));

            light += reduction * (hitdata.emission);

            reduction *= DisneyEval(hitdata, wo, hitdata.normal, wibrdf) * abs(Vector3::dot(wibrdf, hitdata.normal)) / (DisneyPdf(hitdata, wo, hitdata.normal, wibrdf));

            /*

            reduction *= abs(Vector3::dot(ray.direction, hitdata.normal));

            float hdriPdf;
            Vector3 hdriLightCalc = hdriLight(ray, scene, nearestHit.position, hitdata, rnd, hdriPdf);

            light = reduction * hitdata.emission + hdriLightCalc;

            bouncedDir = DisneySample(ray.direction, hitdata, rnd.next(), rnd.next(), rnd.next());

            reduction *= DisneyEval(-1 * bouncedDir, hitdata, -ray.direction) / DisneyPdf(-1 * bouncedDir, hitdata, -ray.direction);

            */

            // First hit
            if (i == 0) {
                normal = hitdata.normal;
                tangent = hitdata.tangent;
                bitangent = hitdata.bitangent;
            }

            ray = Ray(nearestHit.position + wibrdf * 0.001, wibrdf);
        }
        else {
            ray = Ray(nearestHit.position + ray.direction * 0.001, ray.direction);
        }
    }

    // TODO: parametrize light clamp
    light = clamp(light, 0, 10);

    //light = normal;

    if (!sycl::isnan(light.x) && !sycl::isnan(light.y) &&
        !sycl::isnan(light.z)) {
        if (sa > 0) {
            for (int pass = 0; pass < PASSES_COUNT; pass++) {
                if (pass != DENOISE) {
                    scene->dev_passes[(pass * scene->camera->xRes * scene->camera->yRes * 4) + (4 * idx + 0)] *=
                        ((float)sa) / ((float)(sa + 1));
                    scene->dev_passes[(pass * scene->camera->xRes * scene->camera->yRes * 4) + (4 * idx + 1)] *=
                        ((float)sa) / ((float)(sa + 1));
                    scene->dev_passes[(pass * scene->camera->xRes * scene->camera->yRes * 4) + (4 * idx + 2)] *=
                        ((float)sa) / ((float)(sa + 1));
                }
            }
        }

        scene->dev_passes[(BEAUTY * scene->camera->xRes * scene->camera->yRes * 4) + (4 * idx + 0)] +=
            light.x / ((float)sa + 1);
        scene->dev_passes[(BEAUTY * scene->camera->xRes * scene->camera->yRes * 4) + (4 * idx + 1)] +=
            light.y / ((float)sa + 1);
        scene->dev_passes[(BEAUTY * scene->camera->xRes * scene->camera->yRes * 4) + (4 * idx + 2)] +=
            light.z / ((float)sa + 1);

        scene->dev_passes[(NORMAL * scene->camera->xRes * scene->camera->yRes * 4) + (4 * idx + 0)] +=
            normal.x / ((float)sa + 1);
        scene->dev_passes[(NORMAL * scene->camera->xRes * scene->camera->yRes * 4) + (4 * idx + 1)] +=
            normal.y / ((float)sa + 1);
        scene->dev_passes[(NORMAL * scene->camera->xRes * scene->camera->yRes * 4) + (4 * idx + 2)] +=
            normal.z / ((float)sa + 1);

        scene->dev_passes[(TANGENT * scene->camera->xRes * scene->camera->yRes * 4) + (4 * idx + 0)] +=
            tangent.x / ((float)sa + 1);
        scene->dev_passes[(TANGENT * scene->camera->xRes * scene->camera->yRes * 4) + (4 * idx + 1)] +=
            tangent.y / ((float)sa + 1);
        scene->dev_passes[(TANGENT * scene->camera->xRes * scene->camera->yRes * 4) + (4 * idx + 2)] +=
            tangent.z / ((float)sa + 1);

        scene->dev_passes[(BITANGENT * scene->camera->xRes * scene->camera->yRes * 4) + (4 * idx + 0)] +=
            bitangent.x / ((float)sa + 1);
        scene->dev_passes[(BITANGENT * scene->camera->xRes * scene->camera->yRes * 4) + (4 * idx + 1)] +=
            bitangent.y / ((float)sa + 1);
        scene->dev_passes[(BITANGENT * scene->camera->xRes * scene->camera->yRes * 4) + (4 * idx + 2)] +=
            bitangent.z / ((float)sa + 1);

        scene->dev_samples[idx]++;
    }
    scene->dev_randstate[idx] = rnd;
}


void printExp(Exp exp) {
    switch (exp.type) {
    case Exp::Type::NUL:
        std::cout << "EXP(" << exp.idx << "): NUL";
        break;
    case Exp::Type::NUM:
        std::cout << "EXP(" << exp.idx << "): N(" << exp.n << ")";
        break;
    case Exp::Type::VAR:
        std::cout << "EXP(" << exp.idx << "): VAR(" << exp.x << ")";
        break;
    case Exp::Type::VEC:
        std::cout << "EXP(" << exp.idx << "): VEC(";
        printExp(*exp.e1);
        std::cout << ", ";
        printExp(*exp.e2);
        std::cout << ", ";
        printExp(*exp.e3);
        std::cout << ")";
        break;
    case Exp::Type::SUM:
        std::cout << "EXP(" << exp.idx << "): SUM(";
        printExp(*exp.e1);
        std::cout << " + ";
        printExp(*exp.e2);
        std::cout << ")";
        break;
    }
}

void printStatement(Statement sta) {
    switch (sta.type) {
    case Statement::Type::NUL:
        std::cout << "STATEMENT: NUL";
        break;
    case Statement::Type::SKIP:
        std::cout << "STATEMENT: SKIP";
        break;
    case Statement::Type::SEQ:
        std::cout << "STATEMENT: SEQ (";
        printStatement(*sta.s1);
        std::cout << ", ";
        printStatement(*sta.s2);
        std::cout << ")";
        break;
    case Statement::Type::ASS:
        std::cout << "STATEMENT: ASS(" << sta.x << " = ";
        printExp(*sta.e);
        std::cout << ")";
        break;
    case Statement::Type::IF:
        std::cout << "STATEMENT: IF (";
        printStatement(*sta.s1);
        std::cout << ", ";
        printStatement(*sta.s2);
        std::cout << ")";
        break;
    }
}



int renderSetup(sycl::queue& q, Scene* scene, dev_Scene* dev_scene) {

    BOOST_LOG_TRIVIAL(info) << "Initializing rendering";

    dev_Scene* temp = new dev_Scene(scene);

    copy_scene(temp, dev_scene, q);

    BOOST_LOG_TRIVIAL(info) << "OSL MATERIAL TEST";

    OslMaterial* osl = new OslMaterial();

    osl->program = generate_statement();

    BOOST_LOG_TRIVIAL(info) << "PARSED";

    printStatement(*osl->program);

    OslMaterial* dev_osl = sycl::malloc_device<OslMaterial>(1, q);;

    BOOST_LOG_TRIVIAL(info) << "COPYING";

    copy_osl_material(osl, dev_osl, q);

    BOOST_LOG_TRIVIAL(info) << "COPIED";

    int* c = sycl::malloc_device<int>(1, q);

    BOOST_LOG_TRIVIAL(debug) << "Starting setup kernels";

    q.submit([&](cl::sycl::handler& h) {
        sycl::stream out = sycl::stream(4096, 1024, h);
        h.parallel_for(sycl::range(scene->camera.xRes * scene->camera.yRes),
            [=](sycl::id<1> i) {
                setupKernel(dev_scene, i, out, dev_osl);
            });
        }).wait();

    BOOST_LOG_TRIVIAL(info) << "Setup finished";

    //TODO: figure out how to manage max samples (noise cutoff?)


    sycl::range global{ scene->camera.xRes + scene->camera.xRes%8,scene->camera.yRes + scene->camera.yRes % 8 };
    sycl::range local{ 8,8 };

    for (int s = 0; s < 1000; s++) {
        q.submit([&](cl::sycl::handler& h) {
            h.parallel_for(sycl::nd_range{ global, local },
                [=](sycl::nd_item<2> it) {
                    renderingKernel(dev_scene, it.get_global_id(0) * dev_scene->camera->yRes + it.get_global_id(1), s);
                });
            });
    }

    BOOST_LOG_TRIVIAL(info) << "All samples added to the queue";

    return 0;
}