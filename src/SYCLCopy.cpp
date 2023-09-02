#include "SYCLCopy.h"

void copy_scene(dev_Scene* scene, dev_Scene* dev_scene, sycl::queue& q) {

    LOG(debug) << "Copying scene";

    q.memcpy(dev_scene, scene, sizeof(dev_Scene)).wait();

    LOG(debug) << "Copying camera";

    Camera* dev_camera = sycl::malloc_device<Camera>(1, q);
    q.memcpy(dev_camera, scene->camera, sizeof(Camera)).wait();
    q.memcpy(&(dev_scene->camera), &dev_camera, sizeof(Camera*)).wait();

    LOG(debug) << "Copying passes (" << scene->x_res << "x" << scene->y_res << ")";

    float* dev_passes = sycl::malloc_device<float>(PASSES_COUNT * scene->x_res * scene->y_res * 4, q);
    q.memcpy(&(dev_scene->dev_passes), &(dev_passes), sizeof(float*)).wait();
    unsigned int* dev_samples = sycl::malloc_device<unsigned int>(scene->x_res * scene->y_res, q);
    q.memcpy(&(dev_scene->dev_samples), &(dev_samples), sizeof(unsigned int*)).wait();
    RngGenerator* dev_randstate = sycl::malloc_device<RngGenerator>(scene->x_res * scene->y_res, q);
    q.memcpy(&(dev_scene->dev_randstate), &(dev_randstate), sizeof(RngGenerator*)).wait();

    LOG(debug) << "Copying tris";

    Tri* dev_tris = sycl::malloc_device<Tri>(scene->triCount, q);
    q.memcpy(dev_tris, scene->tris, sizeof(Tri) * scene->triCount).wait();
    q.memcpy(&(dev_scene->tris), &(dev_tris), sizeof(Tri*)).wait();

    LOG(debug) << "Copying " << scene->meshObjectCount << " Mesh Objects";

    MeshObject* dev_mo = sycl::malloc_device<MeshObject>(scene->meshObjectCount, q);
    q.memcpy(dev_mo, scene->meshObjects, sizeof(MeshObject) * scene->meshObjectCount).wait();
    q.memcpy(&(dev_scene->meshObjects), &(dev_mo), sizeof(MeshObject*)).wait();
    for (int i = 0; i < scene->meshObjectCount; i++) {
        q.memcpy(&(dev_mo[i].tris), &dev_tris, sizeof(Tri*)).wait();
    }

    LOG(debug) << "Copying " << scene->triCount << " tri - indices";

    int* dev_triIndices = sycl::malloc_device<int>(scene->triCount, q);
    q.memcpy(dev_triIndices, scene->bvh->triIndices, sizeof(int) * scene->triCount).wait();

    LOG(debug) << "Copying BVH";

    BVH* dev_bvh = sycl::malloc_device<BVH>(1, q);
    q.memcpy(dev_bvh, scene->bvh, sizeof(BVH)).wait();
    q.memcpy(&(dev_bvh->tris), &(dev_tris), sizeof(Tri*)).wait();
    q.memcpy(&(dev_bvh->triIndices), &(dev_triIndices), sizeof(int*)).wait();
    q.memcpy(&(dev_scene->bvh), &(dev_bvh), sizeof(BVH*)).wait();

    LOG(debug) << "Copying " << scene->materialCount << " materials";

    Material* dev_materials = sycl::malloc_device<Material>(scene->materialCount, q);
    q.memcpy(dev_materials, scene->materials, sizeof(Material) * scene->materialCount).wait();
    q.memcpy(&(dev_scene->materials), &(dev_materials), sizeof(Material*)).wait();

    for (int i = 0; i < scene->materialCount; i++) {
        LOG(debug) << "" << scene->materials[i].to_string();
    }

    LOG(debug) << "Copying " << scene->pointLightCount << " pointlights";

    PointLight* dev_pointLights = sycl::malloc_device<PointLight>(scene->pointLightCount, q);
    q.memcpy(dev_pointLights, scene->pointLights, sizeof(PointLight) * scene->pointLightCount).wait();
    q.memcpy(&(dev_scene->pointLights), &(dev_pointLights), sizeof(PointLight*)).wait();


    LOG(debug) << "Copying " << scene->textureCount << " textures to GPU";

    Texture* dev_textures = sycl::malloc_device<Texture>(scene->textureCount, q);

    q.memcpy(dev_textures, scene->textures, sizeof(Texture) * scene->textureCount).wait();

    for (int i = 0; i < scene->textureCount; i++) {
        LOG(debug) << scene->textures[i].to_string();
        float* textureData = sycl::malloc_device<float>(scene->textures[i].width * scene->textures[i].height * scene->textures[i].channels, q);
        q.memcpy(textureData, scene->textures[i].data, sizeof(float) * scene->textures[i].width * scene->textures[i].height * scene->textures[i].channels).wait();
        q.memcpy(&(dev_textures[i].data), &textureData, sizeof(float*)).wait();
    }

    q.memcpy(&(dev_scene->textures), &(dev_textures), sizeof(Texture*)).wait();


    LOG(debug) << "Copying HDRI";

    HDRI* hdri = scene->hdri;
    HDRI* dev_hdri = sycl::malloc_device<HDRI>(1, q);

    float* dev_data = sycl::malloc_device<float>(hdri->texture.height * hdri->texture.width * hdri->texture.channels, q);
    float* dev_cdf = sycl::malloc_device<float>(hdri->texture.height * hdri->texture.width + 1, q);

    q.memcpy(dev_hdri, hdri, sizeof(HDRI)).wait();
    q.memcpy(dev_data, hdri->texture.data, sizeof(float) * hdri->texture.height * hdri->texture.width * hdri->texture.channels).wait();
    q.memcpy(dev_cdf, hdri->cdf, sizeof(float) * hdri->texture.height * hdri->texture.width + 1).wait();

    q.memcpy((&dev_hdri->texture.data), &(dev_data), sizeof(float*)).wait();
    q.memcpy(&(dev_hdri->cdf), &(dev_cdf), sizeof(float*)).wait();
    q.memcpy(&(dev_scene->hdri), &(dev_hdri), sizeof(float*)).wait();


    LOG(info) << "All scene copied";

}
