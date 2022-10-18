#include "SYCLCopy.h"

void copy_exp(Exp* exp, Exp* dev_exp, sycl::queue& q) {

    Exp* dev_exp1;
    Exp* dev_exp2;
    Exp* dev_exp3;

    q.memcpy(&(dev_exp->type), &(exp->type), sizeof(Exp::Type)).wait();
    //q.memcpy(&(dev_exp->Atemp_val), &(exp->Atemp_val), sizeof(Var)).wait();
    q.memcpy(&(dev_exp->Vtemp_val), &(exp->Vtemp_val), sizeof(Var)).wait();
    //q.memcpy(&(dev_exp->Avisited), &(exp->Avisited), sizeof(bool)).wait();
    //q.memcpy(&(dev_exp->At_visited), &(exp->At_visited), sizeof(bool)).wait();
    q.memcpy(&(dev_exp->idx), &(exp->idx), sizeof(int)).wait();
    q.memcpy(&(dev_exp->Vvisited), &(exp->Vvisited), sizeof(bool)).wait();
    q.memcpy(&(dev_exp->Vt_visited), &(exp->Vt_visited), sizeof(bool)).wait();

    switch (exp->type) {
    case Exp::Type::NUM:
        q.memcpy(&(dev_exp->n), &(exp->n), sizeof(float)).wait();
        break;
    case Exp::Type::VEC:
        dev_exp1 = sycl::malloc_device<Exp>(1, q);
        dev_exp2 = sycl::malloc_device<Exp>(1, q);
        dev_exp3 = sycl::malloc_device<Exp>(1, q);
        copy_exp(exp->e1, dev_exp1, q);
        copy_exp(exp->e2, dev_exp2, q);
        copy_exp(exp->e3, dev_exp3, q);
        q.memcpy(&(dev_exp->e1), &dev_exp1, sizeof(Exp*)).wait();
        q.memcpy(&(dev_exp->e2), &dev_exp2, sizeof(Exp*)).wait();
        q.memcpy(&(dev_exp->e3), &dev_exp3, sizeof(Exp*)).wait();
        break;
    case Exp::Type::VAR:
        q.memcpy(&(dev_exp->x), &(exp->x), sizeof(strlen(exp->x) + 1)).wait();
        break;
    case Exp::Type::SUM:
        dev_exp1 = sycl::malloc_device<Exp>(1, q);
        dev_exp2 = sycl::malloc_device<Exp>(1, q);
        copy_exp(exp->e1, dev_exp1, q);
        copy_exp(exp->e2, dev_exp2, q);
        q.memcpy(&(dev_exp->e1), &dev_exp1, sizeof(Exp*)).wait();
        q.memcpy(&(dev_exp->e2), &dev_exp2, sizeof(Exp*)).wait();
        break;
    }
}

void copy_statement(Statement* sta, Statement* dev_sta, sycl::queue& q) {

    q.memcpy(&(dev_sta->type), &(sta->type), sizeof(Statement::Type)).wait();

    Exp* dev_exp;
    char* dev_var;
    Statement* dev_sta1;
    Statement* dev_sta2;

    switch (sta->type) {
    case Statement::Type::SEQ:
        dev_sta1 = sycl::malloc_device<Statement>(1, q);
        dev_sta2 = sycl::malloc_device<Statement>(1, q);
        copy_statement(sta->s1, dev_sta1, q);
        copy_statement(sta->s2, dev_sta2, q);
        q.memcpy(&(dev_sta->s1), &dev_sta1, sizeof(Statement*)).wait();
        q.memcpy(&(dev_sta->s2), &dev_sta2, sizeof(Statement*)).wait();
        break;
    case Statement::Type::ASS:
        dev_exp = sycl::malloc_device<Exp>(1, q);
        copy_exp(sta->e, dev_exp, q);
        q.memcpy(&(dev_sta->e), &dev_exp, sizeof(Exp*)).wait();
        q.memcpy(&(dev_sta->x), &(sta->x), sizeof(strlen(sta->x) + 1)).wait();
        break;
    case Statement::Type::SKIP:
        break;
    case Statement::Type::IF:
        dev_exp = sycl::malloc_device<Exp>(1, q);
        copy_exp(sta->e, dev_exp, q);
        Statement* dev_sta1 = sycl::malloc_device<Statement>(1, q);
        Statement* dev_sta2 = sycl::malloc_device<Statement>(1, q);
        copy_statement(sta->s1, dev_sta1, q);
        copy_statement(sta->s2, dev_sta2, q);
        q.memcpy(&(dev_sta->e), &dev_exp, sizeof(Exp*)).wait();
        q.memcpy(&(dev_sta->s1), &dev_sta1, sizeof(Statement*)).wait();
        q.memcpy(&(dev_sta->s2), &dev_sta2, sizeof(Statement*)).wait();
        break;
    }
}

void copy_osl_material(OslMaterial* mat, OslMaterial* dev_mat, sycl::queue& q) {

    Statement* dev_sta = sycl::malloc_device<Statement>(1, q);

    copy_statement(mat->program, dev_sta, q);

    q.memcpy(&(dev_mat->program), &dev_sta, sizeof(Statement*)).wait();
}

void copy_scene(dev_Scene* scene, dev_Scene* dev_scene, sycl::queue& q) {

    BOOST_LOG_TRIVIAL(debug) << "Copying scene";

    q.memcpy(dev_scene, scene, sizeof(dev_Scene)).wait();

    BOOST_LOG_TRIVIAL(debug) << "Copying camera";

    Camera* dev_camera = sycl::malloc_device<Camera>(1, q);
    q.memcpy(dev_camera, scene->camera, sizeof(Camera)).wait();
    q.memcpy(&(dev_scene->camera), &dev_camera, sizeof(Camera*)).wait();

    BOOST_LOG_TRIVIAL(debug) << "Copying passes";

    float* dev_passes = sycl::malloc_device<float>(PASSES_COUNT * scene->camera->xRes * scene->camera->yRes * 4, q);
    q.memcpy(&(dev_scene->dev_passes), &(dev_passes), sizeof(float*)).wait();
    unsigned int* dev_samples = sycl::malloc_device<unsigned int>(scene->camera->xRes * scene->camera->yRes, q);
    q.memcpy(&(dev_scene->dev_samples), &(dev_samples), sizeof(unsigned int*)).wait();
    RngGenerator* dev_randstate = sycl::malloc_device<RngGenerator>(scene->camera->xRes * scene->camera->yRes, q);
    q.memcpy(&(dev_scene->dev_randstate), &(dev_randstate), sizeof(RngGenerator*)).wait();

    BOOST_LOG_TRIVIAL(debug) << "Copying tris";

    Tri* dev_tris = sycl::malloc_device<Tri>(scene->triCount, q);
    q.memcpy(dev_tris, scene->tris, sizeof(Tri) * scene->triCount).wait();
    q.memcpy(&(dev_scene->tris), &(dev_tris), sizeof(Tri*)).wait();

    BOOST_LOG_TRIVIAL(debug) << "Copying Mesh Objects";

    MeshObject* dev_mo = sycl::malloc_device<MeshObject>(scene->meshObjectCount, q);
    q.memcpy(dev_mo, scene->meshObjects, sizeof(MeshObject) * scene->meshObjectCount).wait();
    q.memcpy(&(dev_scene->meshObjects), &(dev_mo), sizeof(MeshObject*)).wait();
    for (int i = 0; i < scene->meshObjectCount; i++) {
        q.memcpy(&(dev_mo[i].tris), &dev_tris, sizeof(Tri*)).wait();
    }

    BOOST_LOG_TRIVIAL(debug) << "Copying tri-indices";

    int* dev_triIndices = sycl::malloc_device<int>(scene->triCount, q);
    q.memcpy(dev_triIndices, scene->bvh->triIndices, sizeof(int) * scene->triCount).wait();

    BOOST_LOG_TRIVIAL(debug) << "Copying BVH";

    BVH* dev_bvh = sycl::malloc_device<BVH>(1, q);
    q.memcpy(dev_bvh, scene->bvh, sizeof(BVH)).wait();
    q.memcpy(&(dev_bvh->tris), &(dev_tris), sizeof(Tri*)).wait();
    q.memcpy(&(dev_bvh->triIndices), &(dev_triIndices), sizeof(int*)).wait();
    q.memcpy(&(dev_scene->bvh), &(dev_bvh), sizeof(BVH*)).wait();

    BOOST_LOG_TRIVIAL(debug) << "Copying " << scene->materialCount << " materials";

    Material* dev_materials = sycl::malloc_device<Material>(scene->materialCount, q);
    q.memcpy(dev_materials, scene->materials, sizeof(Material) * scene->materialCount).wait();
    q.memcpy(&(dev_scene->materials), &(dev_materials), sizeof(Material*)).wait();

    BOOST_LOG_TRIVIAL(debug) << "Copying " << scene->pointLightCount << " pointlights";

    PointLight* dev_pointLights = sycl::malloc_device<PointLight>(scene->pointLightCount, q);
    q.memcpy(dev_pointLights, scene->pointLights, sizeof(PointLight) * scene->pointLightCount).wait();
    q.memcpy(&(dev_scene->pointLights), &(dev_pointLights), sizeof(PointLight*)).wait();


    BOOST_LOG_TRIVIAL(debug) << "Copying " << scene->textureCount << " textures to GPU";

    Texture* dev_textures = sycl::malloc_device<Texture>(scene->textureCount, q);

    q.memcpy(dev_textures, scene->textures, sizeof(Texture) * scene->textureCount).wait();

    for (int i = 0; i < scene->textureCount; i++) {
        float* textureData = sycl::malloc_device<float>(scene->textures[i].width * scene->textures[i].height * 3, q);
        q.memcpy(textureData, scene->textures[i].data, sizeof(float) * scene->textures[i].width * scene->textures[i].height * 3).wait();
        q.memcpy(&(dev_textures[i].data), &textureData, sizeof(float*)).wait();
    }

    q.memcpy(&(dev_scene->textures), &(dev_textures), sizeof(Texture*)).wait();


    BOOST_LOG_TRIVIAL(debug) << "Copying HDRI to GPU";

    HDRI* hdri = scene->hdri;
    HDRI* dev_hdri = sycl::malloc_device<HDRI>(1, q);

    float* dev_data = sycl::malloc_device<float>(hdri->texture.height * hdri->texture.width * 3, q);
    float* dev_cdf = sycl::malloc_device<float>(hdri->texture.height * hdri->texture.width, q);

    q.memcpy(dev_hdri, hdri, sizeof(HDRI)).wait();
    q.memcpy(dev_data, hdri->texture.data, sizeof(float) * hdri->texture.height * hdri->texture.width * 3).wait();
    q.memcpy(dev_cdf, hdri->cdf, sizeof(float) * hdri->texture.height * hdri->texture.width).wait();

    q.memcpy((&dev_hdri->texture.data), &(dev_data), sizeof(float*)).wait();
    q.memcpy(&(dev_hdri->cdf), &(dev_cdf), sizeof(float*)).wait();
    q.memcpy(&(dev_scene->hdri), &(dev_hdri), sizeof(float*)).wait();

}





/*
// TODO: abstracting SYCL copy


template <typename T>
struct SYCLObject {

    enum class DataType {
        INT, INT_PTR, FLOAT, FLOAT_PTR, EXP, EXP_PTR, STATEMENT, STATEMENT_PTR
    };

    int sizeof_type(DataType type) {
        if (type == DataType::INT)
            return sizeof(int);
        if (type == DataType::FLOAT)
            return sizeof(float);
        if (type == DataType::EXP)
            return sizeof(Exp);
        if (type == DataType::STATEMENT)
            return sizeof(Statement);

        //Ptr size
        return 8;
    }


    std::vector<std::tuple<void* T::*, DataType>> fields;
    std::vector<std::tuple<void* T::*, DataType>> ptrs;

    template<typename D>
    void register_field(D T::* field, DataType type) {
        fields.push_back(std::tuple<void* T::*, DataType>((void* T::*)field, type));
    }

    template<typename D>
    void register_ptr(D T::* field, DataType type) {
        fields.push_back(std::tuple<void* T::*, DataType>((void* T::*)field, type));
    }

    void copy(T& obj, T& dev_obj, sycl::queue& q) {

        // Copy datafields
        for (std::tuple<T::*, int> field_tuple : fields) {
            T::* field_ptr = std::get<0>(field_tuple);
            DataType field_type = std::get<1>(field_tuple);

            q.memcpy(&dev_obj::field_ptr, &obj::field_ptr, sizeof_type(field_size)).wait();
        }

        // Copy ptrs recursively
        for (std::tuple<T::*, int> field_tuple : ptrs) {
            T::* field_ptr = std::get<0>(field_tuple);
            DataType field_type = std::get<1>(field_tuple);

            q.memcpy(&dev_obj::field_ptr, &obj::field_ptr, field_size).wait();
        }
    }

};

struct SYCLManager {


    enum class DataType {
        INT, INT_PTR, FLOAT, FLOAT_PTR, EXP, EXP_PTR, STATEMENT, STATEMENT_PTR
    };


    std::vector<DataType> types;


    void register_type(DataType type) {

    }

    void register_field() {

    }

    void register_data_field() {

    }

    void copy() {

    }

    void setup() {

        // registro el tipo exp
        //register_type(Exp);

        // a�ado fields de expr
        //register_data_field(Exp, &Exp::idx);

        // a�ado el puntero a otra exp
        //register_ptr_field(Exp, &Exp::e1);


        //register_arr_field(Exp, &Exp::x, 16 * sizeof(char));

    }


};




*/