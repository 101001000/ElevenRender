#include <cassert>

#include "mikktspace.h"
#include "mikktspaceCallback.h"
#include "MeshObject.hpp"
#include "Definitions.h"

//Adaptation from
//https://www.turais.de/using-mikktspace-in-your-project/


CalcTangents::CalcTangents() {

    iface.m_getNumFaces = get_num_faces;
    iface.m_getNumVerticesOfFace = get_num_vertices_of_face;

    iface.m_getNormal = get_normal;
    iface.m_getPosition = get_position;
    iface.m_getTexCoord = get_tex_coords;
    iface.m_setTSpaceBasic = set_tspace_basic;

    context.m_pInterface = &iface;
}

void CalcTangents::calc(MeshObject* mesh) {

    context.m_pUserData = mesh;

    genTangSpaceDefault(&this->context);
}

int CalcTangents::get_num_faces(const SMikkTSpaceContext* context) {

    MeshObject* working_mesh = static_cast<MeshObject*> (context->m_pUserData);

    return working_mesh->triCount;

}

int CalcTangents::get_num_vertices_of_face(const SMikkTSpaceContext* context,
    const int iFace) {
    return 3;
}

void CalcTangents::get_position(const SMikkTSpaceContext* context, float outpos[],
    int iFace, int iVert) {

    MeshObject* working_mesh = static_cast<MeshObject*> (context->m_pUserData);

    outpos[0] = working_mesh->tris[iFace].vertices[iVert].x;
    outpos[1] = working_mesh->tris[iFace].vertices[iVert].y;
    outpos[2] = working_mesh->tris[iFace].vertices[iVert].z;
}

void CalcTangents::get_normal(const SMikkTSpaceContext* context, float outnormal[],
    int iFace, int iVert) {
    MeshObject* working_mesh = static_cast<MeshObject*> (context->m_pUserData);

 #if SMOOTH_SHADING 

    outnormal[0] = working_mesh->tris[iFace].normals[iVert].x;
    outnormal[1] = working_mesh->tris[iFace].normals[iVert].y;
    outnormal[2] = working_mesh->tris[iFace].normals[iVert].z;

#else

    Vector3 edge1 = working_mesh->tris[iFace].vertices[1] - working_mesh->tris[iFace].vertices[0];
    Vector3 edge2 = working_mesh->tris[iFace].vertices[2] - working_mesh->tris[iFace].vertices[0];

    Vector3 N = Vector3::cross(edge1, edge2).normalized();

    outnormal[0] = N.x;
    outnormal[1] = N.y;
    outnormal[2] = N.z;

#endif

}

void CalcTangents::get_tex_coords(const SMikkTSpaceContext* context,
    float outuv[],
    const int iFace, const int iVert) {

    MeshObject* working_mesh = static_cast<MeshObject*> (context->m_pUserData);

    outuv[0] = working_mesh->tris[iFace].uv[iVert].x;
    outuv[1] = working_mesh->tris[iFace].uv[iVert].y;
}

void CalcTangents::set_tspace_basic(const SMikkTSpaceContext* context,
    const float tangentu[],
    const float fSign, const int iFace, const int iVert) {
    MeshObject* working_mesh = static_cast<MeshObject*> (context->m_pUserData);

    working_mesh->tris[iFace].tangents[iVert] = Vector3(tangentu[0], tangentu[1], tangentu[2]);
    working_mesh->tris[iFace].tangentsSign = fSign;
}