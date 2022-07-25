#pragma once

#include <limits>
#include "Tri.hpp"
#include "Math.hpp"
#include "Ray.hpp"
#include <vector>
#include <algorithm>
#include <chrono>  
#include "Definitions.h"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#include <CL/sycl.hpp>
#pragma clang diagnostic pop


struct BVHTri {

	Tri tri;
	int index;

	BVHTri(Tri _tri, int _index);

	BVHTri();

};

struct Node {

	Vector3 b1, b2;
	int from, to, idx, depth;
	bool valid;

	Node();

	Node(int _idx, Vector3 _b1, Vector3 _b2, int _from, int _to, int _depth);
};

// Data structure which holds all the geometry data organized so it can be intersected fast with light rays.

class BVH {

public:

	int nodeIdx = 0;
	int allocatedTris = 0;
	int totalTris = 0;

	Tri* tris;

	Node nodes[2 << BVH_DEPTH];

	int triIdx = 0;
	int* triIndices;

	BVH() {	}

	bool intersect(Ray ray, Vector3 b1, Vector3 b2);

	void transverseAux(Ray ray, Hit& nearestHit, Node& node);

	SYCL_EXTERNAL void transverse(Ray ray, Hit& nearestHit);

	void intersectNode(Ray ray, Node node, Hit& nearestHit);

	Node leftChild(int idx, int depth);

	Node rightChild(int idx, int depth);

	void build(std::vector<Tri>* _fullTris);

	void buildIt(std::vector<BVHTri>* _tris);

	void buildAux(int depth, std::vector<BVHTri>* _tris);

	static void dividePlane(std::vector<BVHTri>* tris, std::vector<BVHTri>* trisLeft, std::vector<BVHTri>* trisRight);

	static void divideSAH(std::vector<BVHTri>* tris, std::vector<BVHTri>* trisLeft, std::vector<BVHTri>* trisRight);

	static void divideNaive(std::vector<BVHTri>* tris, std::vector<BVHTri>* trisLeft, std::vector<BVHTri>* trisRight);

	static void boundsUnion(Vector3 b1, Vector3 b2, Vector3 b3, Vector3 b4, Vector3& b5, Vector3& b6);

	static float boundsArea(Vector3 b1, Vector3 b2);

	static void bounds(Tri tri, Vector3& b1, Vector3& b2);

	static void bounds(std::vector<BVHTri>* tris, Vector3& b1, Vector3& b2);

};


