#pragma once

#include "Hit.h"
#include "Ray.h"
#include "Definitions.h"


class Tri {
public:

    Vector3 vertices[3];
    Vector3 uv[3];
    Vector3 normals[3];
    Vector3 tangents[3];

    float tangentsSign;

    int objectID;
    int materialID;

    std::string matName;


public:

    Tri() {

    }

    inline Vector3 centroid() {

        return Vector3(vertices[0].x + vertices[1].x + vertices[2].x,
            vertices[0].y + vertices[1].y + vertices[2].y,
            vertices[0].z + vertices[1].z + vertices[2].z) / 3.0;
    }

    inline Vector3 projectOnPlane(Vector3 position, Vector3 origin, Vector3 normal) {
        return position - Vector3::dot(position - origin, normal) * normal;
    }

    inline bool hit(Ray& ray, Hit& hit) {

        float EPSILON = 0.0000001f;

        Vector3 edge1 = vertices[1] - vertices[0];
        Vector3 edge2 = vertices[2] - vertices[0];

        Vector3 pvec = Vector3::cross(ray.direction, edge2);

        float u, v, t, inv_det;

        float det = Vector3::dot(edge1, pvec);

        inv_det = 1.0f / det;

        if (det > -EPSILON && det < EPSILON) return false;

        Vector3 tvec = ray.origin - vertices[0];

        u = Vector3::dot(tvec, pvec) * inv_det;
        if (u < 0 || u > 1) {
            return false;
        }

        Vector3 qvec = Vector3::cross(tvec, edge1);
        v = Vector3::dot(ray.direction, qvec) * inv_det;
        if (v < 0 || (u + v) > 1) {
            return false;
        }

        t = Vector3::dot(edge2, qvec) * inv_det;

        if (t < 0) { return false; }

        // UV coordinates from the texture, weighted with vertex texture UV
        Vector3 tUV = uv[0] + (uv[1] - uv[0]) * u + (uv[2] - uv[0]) * v;
        Vector3 geomPosition = ray.origin + ray.direction * t;

#if SMOOTH_SHADING 

        // https://gist.github.com/pixnblox/5e64b0724c186313bc7b6ce096b08820

        //Vector3 n0 = Vector3::dot(normals[0], ray.direction) > 0 ? -normals[0] : normals[0];
        //Vector3 n1 = Vector3::dot(normals[1], ray.direction) > 0 ? -normals[1] : normals[1];
        //Vector3 n2 = Vector3::dot(normals[2], ray.direction) > 0 ? -normals[2] : normals[2];


        Vector3 n0 = normals[0];
        Vector3 n1 = normals[1];
        Vector3 n2 = normals[2];

        Vector3 shadingNormal = (n0 + (n1 - n0) * u + (n2 - n0) * v).normalized();

        Vector3 compNormal = Vector3::cross(edge1, edge2).normalized();

        if (Vector3::dot(compNormal, ray.direction) > 0) {
            compNormal *= -1;
        }

        //Maybe I need to flip tangents
        Vector3 shadingTangent = tangents[0] + (tangents[1] - tangents[0]) * u + (tangents[2] - tangents[0]) * v;


        //Shadow terminator creating artifacts
        
        Vector3 p0 = projectOnPlane(geomPosition, vertices[0], n0);
        Vector3 p1 = projectOnPlane(geomPosition, vertices[1], n1);
        Vector3 p2 = projectOnPlane(geomPosition, vertices[2], n2);

        Vector3 shadingPosition = p0 + (p1 - p0) * u + (p2 - p0) * v;

        bool convex = Vector3::dot(shadingPosition - geomPosition, shadingNormal) > 0;
        
        //Some objects have bad normals, as a quick fix, I'm just using the calculated normal unless the one from the vertex is similar to the calculated one.
        hit.tangent = shadingTangent;
        //hit.position = geomPosition;
        hit.position = convex ? shadingPosition : geomPosition;
        //hit.normal = abs(Vector3::dot(shadingNormal, compNormal)) > 0.9 ? shadingNormal : compNormal;
        hit.normal = shadingNormal;
        hit.gnormal = compNormal;

#else
        Vector3 geomNormal = Vector3::cross(edge1, edge2).normalized();

        if (Vector3::dot(geomNormal, ray.direction) > 0) {
            geomNormal *= -1;
        }

 
        // tangents[0], tangents[1], tangents[2] are the same when smoothshading is off
        hit.tangent = tangents[0];
        hit.normal = geomNormal;
        hit.position = geomPosition;
#endif
     
        hit.bitangent = tangentsSign * Vector3::cross(hit.normal, hit.tangent);
        hit.valid = true;
        hit.tu = tUV.x;
        hit.tv = tUV.y;
        hit.objectID = objectID;
        hit.material_id = materialID;

        return true;
    }
};

 inline bool operator==(const Tri& t1, const Tri& t2) {
    return (t1.uv[0] == t2.uv[0]) &&
        (t1.uv[1] == t2.uv[1]) &&
        (t1.uv[2] == t2.uv[2]) &&
        (t1.vertices[0] == t2.vertices[0]) &&
        (t1.vertices[1] == t2.vertices[1]) &&
        (t1.vertices[2] == t2.vertices[2]);
}
