#include "BVH.h"


BVHTri::BVHTri(Tri _tri, int _index) {
	tri = _tri;
	index = _index;
}


Node::Node() {
	valid = false;
}

Node::Node(int _idx, Vector3 _b1, Vector3 _b2, int _from, int _to, int _depth) {
	idx = _idx;
	b1 = _b1;
	b2 = _b2;
	from = _from;
	to = _to;
	valid = true;
	depth = _depth;
}




bool BVH::intersect(Ray ray, Vector3 b1, Vector3 b2) {

	// Direct implementation of https://gamedev.stackexchange.com/questions/18436/most-efficient-aabb-vs-ray-collision-algorithms
	Vector3 dirfrac;

	// r.dir is unit direction vector of ray
	dirfrac.x = 1.0f / ray.direction.x;
	dirfrac.y = 1.0f / ray.direction.y;
	dirfrac.z = 1.0f / ray.direction.z;

	// lb is the corner of AABB with minimal coordinates - left bottom, rt is maximal corner
	// r.org is origin of ray
	float t1 = (b1.x - ray.origin.x) * dirfrac.x;
	float t2 = (b2.x - ray.origin.x) * dirfrac.x;
	float t3 = (b1.y - ray.origin.y) * dirfrac.y;
	float t4 = (b2.y - ray.origin.y) * dirfrac.y;
	float t5 = (b1.z - ray.origin.z) * dirfrac.z;
	float t6 = (b2.z - ray.origin.z) * dirfrac.z;

	float tmin = maxf(maxf(minf(t1, t2), minf(t3, t4)), minf(t5, t6));
	float tmax = minf(minf(maxf(t1, t2), maxf(t3, t4)), maxf(t5, t6));

	// if tmax < 0, ray (line) is intersecting AABB, but the whole AABB is behind us
	if (tmax < 0)
		return false;

	// if tmin > tmax, ray doesn't intersect AABB
	if (tmin > tmax)
		return false;

	return true;

}

void BVH::transverseAux(Ray ray, Hit& nearestHit, Node& node) {
	
	if (node.depth == BVH_DEPTH) {
		intersectNode(ray, node, nearestHit);
		return;
	}

	Node lChild = leftChild(node.idx, node.depth);
	Node rChild = rightChild(node.idx, node.depth);

	if (intersect(ray, lChild.b1, lChild.b2))
		transverseAux(ray, nearestHit, lChild);
	
	if (intersect(ray, rChild.b1, rChild.b2))
		transverseAux(ray, nearestHit, rChild);		
}

void BVH::transverse(Ray ray, Hit& nearestHit) {
  
	Node stack[64];

	Node* stackPtr = stack;

	(*stackPtr++).valid = false;

	Node node = nodes[0];

	do {

		Node lChild = leftChild(node.idx, node.depth);
		Node rChild = rightChild(node.idx, node.depth);

		bool lOverlap = intersect(ray, lChild.b1, lChild.b2);
		bool rOverlap = intersect(ray, rChild.b1, rChild.b2);
         
		if (node.depth == (BVH_DEPTH - 1) && rOverlap)
			intersectNode(ray, rChild, nearestHit);

		if (node.depth == (BVH_DEPTH - 1) && lOverlap)
			intersectNode(ray, lChild, nearestHit);

		bool traverseL = (lOverlap && node.depth != (BVH_DEPTH - 1));
		bool traverseR = (rOverlap && node.depth != (BVH_DEPTH - 1));

		if (!traverseL && !traverseR) {
			node = *--stackPtr;

		} else {
			node = (traverseL) ? lChild : rChild;
			if (traverseL && traverseR)
				*stackPtr++ = rChild;
		}

	} while (node.valid);
}

void BVH::intersectNode(Ray ray, Node node, Hit& nearestHit) {

	for (int i = node.from; i < node.to; i++) {

		Hit hit;
        
		if (tris[triIndices[i]].hit(ray, hit)) {
             
			if (!nearestHit.valid) {
				nearestHit = hit;
			}
			else if ((hit.position - ray.origin).length() < (nearestHit.position - ray.origin).length()) {
				nearestHit = hit;
			}
		}
	}
}

Node BVH::leftChild(int idx, int depth) {
	if (depth == BVH_DEPTH) return Node();
	return nodes[idx + 1];
}

Node BVH::rightChild(int idx, int depth) {
	if (depth == BVH_DEPTH) return Node();
	return nodes[idx + (2 << (BVH_DEPTH - depth - 1))];
}

void BVH::build(std::vector<Tri>* _fullTris) {

	std::vector<BVHTri>* _tris = new std::vector<BVHTri>();

	for (int i = 0; i < _fullTris->size(); i++) {
		BVHTri bvhTri(_fullTris->at(i), i);
		_tris->push_back(bvhTri);
	}			

	buildAux(0, _tris);

	std::vector<Tri>* sortedTris = new std::vector<Tri>();

	for (int i = 0; i < _tris->size(); i++) {
		sortedTris->push_back(_fullTris->at(triIndices[i]));
	}

	for (int i = 0; i < _tris->size(); i++) {
		Tri t = sortedTris->at(i);
		//_fullTris->data()[i] = t;
	}

	delete(_tris);
	delete(sortedTris);
}

void BVH::buildIt(std::vector<BVHTri>* _tris) {

	struct BuildNode {
		Node node;
		std::vector<BVHTri>* tris;

		BuildNode(Node _node, std::vector<BVHTri>* _tris) {
			node = _node;
			tris = _tris;
		}

		BuildNode() {

		}
	};

	BuildNode stack[64];

	int stackPointer = 0;

	Vector3 b1, b2, b3, b4;

	bounds(_tris, b1, b2);

	stack[stackPointer++] = BuildNode(Node(-1, b1, b2, 0, 0, 0), _tris);

	while (stackPointer > 0) {

		// POP
		BuildNode node = stack[--stackPointer];

		node.node.idx = nodeIdx++;

		if (node.node.depth == 0)
			totalTris = node.tris->size();

		if (node.node.depth == 7)
			printf("\rAllocated tris: %d / %d, %d%%", allocatedTris, totalTris, (100 * allocatedTris) / totalTris);

		// Nodo hoja
		if (node.node.depth == BVH_DEPTH) {

			node.node.from = triIdx;
			node.node.to = triIdx + node.tris->size();

			for (int i = 0; i < node.tris->size(); i++)
				triIndices[triIdx++] = node.tris->at(i).index;

			allocatedTris += node.tris->size();
		}
		else {

			std::vector<BVHTri>* trisLeft = new std::vector<BVHTri>();
			std::vector<BVHTri>* trisRight = new std::vector<BVHTri>();

			divideSAH(node.tris, trisLeft, trisRight);

			bounds(trisLeft, b1, b2);
			bounds(trisRight, b3, b4);

			BuildNode leftNode = BuildNode(Node(-1, b1, b2, 0, 0, node.node.depth + 1), trisLeft);
			BuildNode rightNode = BuildNode(Node(-1, b3, b4, 0, 0, node.node.depth + 1), trisRight);

			stack[stackPointer++] = rightNode;
			stack[stackPointer++] = leftNode;

			delete(trisLeft);
			delete(trisRight);
		}

		node.tris->clear();

		delete node.tris;

		nodes[node.node.idx] = node.node;
	}
}
	
void BVH::buildAux(int depth, std::vector<BVHTri>* _tris) {

	Vector3 b1, b2;

	if (depth == 0)
		totalTris = _tris->size();

	if(depth == 7)
		printf("\rAllocated tris: %d / %d, %d%%", allocatedTris, totalTris, (100 * allocatedTris) / totalTris);

	bounds(_tris, b1, b2);

	if (depth == BVH_DEPTH) {

		nodes[nodeIdx++] = Node(nodeIdx, b1, b2, triIdx, triIdx + _tris->size(), depth);

		for (int i = 0; i < _tris->size(); i++)
			triIndices[triIdx++] = _tris->at(i).index;

		allocatedTris += _tris->size();
	}
	else {

		nodes[nodeIdx++] = Node(nodeIdx, b1, b2, 0, 0, depth);

		std::vector<BVHTri>* trisLeft = new std::vector<BVHTri>();
		std::vector<BVHTri>* trisRight = new std::vector<BVHTri>();

		divideSAH(_tris, trisLeft, trisRight);

		buildAux(depth + 1, trisLeft);
		buildAux(depth + 1, trisRight);

		trisLeft->clear();
		trisRight->clear();

		delete(trisLeft);
		delete(trisRight);
	}
}

/*

void BVH::dividePlane(std::vector<BVHTri>* tris, std::vector<BVHTri>* trisLeft, std::vector<BVHTri>* trisRight) {

	Vector3 b1, b2;

	bounds(tris, b1, b2);

	Vector3 l = (b2.x - b1.x, b2.y - b1.y, b2.z - b1.z);

	if (l.x > l.y && l.x > l.z) {

		for (int i = 0; i < tris->size(); i++) {
			if (tris->at(i).tri.centroid().x > b1.x + l.x / 2) {
				trisLeft->push_back(tris->at(i));
			}
			else {
				trisRight->push_back(tris->at(i));
			}
		}

	} else if (l.y > l.x && l.y > l.z) {
		for (int i = 0; i < tris->size(); i++) {
			if (tris->at(i).tri.centroid().y > b1.y + l.y / 2) {
				trisLeft->push_back(tris->at(i));
			}
			else {
				trisRight->push_back(tris->at(i));
			}
		}
	}
	else {
		for (int i = 0; i < tris->size(); i++) {
			if (tris->at(i).tri.centroid().z > b1.z + l.z / 2) {
				trisLeft->push_back(tris->at(i));
			}
			else {
				trisRight->push_back(tris->at(i));
			}
		}
	}
}
*/

void BVH::divideSAH(std::vector<BVHTri>* tris, std::vector<BVHTri>* trisLeft, std::vector<BVHTri>* trisRight) {

	if (tris->size() <= 0)
		return;

	Vector3 totalB1, totalB2;

	int bestBin = 0;
	int bestAxis = 0;
	float bestHeuristic = std::numeric_limits<float>::max();

	bounds(tris, totalB1, totalB2);

	for (int axis = 0; axis < 3; axis++) {

		Vector3 b1s[BVH_SAHBINS];
		Vector3 b2s[BVH_SAHBINS];

		int count[BVH_SAHBINS];

		// Bin initialization
		for (int i = 0; i < BVH_SAHBINS; i++) {
			count[i] = 0;
			b1s[i] = Vector3();
			b2s[i] = Vector3();
		}

		// Bin filling
		for (int i = 0; i < tris->size(); i++) {

			int bin = 0;
			Vector3 b1, b2;

			// The bin which corresponds to certain triangle is calculated
			if (totalB1[axis] != totalB2[axis]) {
				float c = tris->at(i).tri.centroid()[axis];
				bin = map(c, totalB1[axis], totalB2[axis], 0, BVH_SAHBINS - 1);
			}

			count[bin]++;
			bounds(tris->at(i).tri, b1, b2);
			boundsUnion(b1s[bin], b2s[bin], b1, b2, b1s[bin], b2s[bin]);
		}
		
		for (int i = 0; i < BVH_SAHBINS; i++) {

			// Tris in the first and second child
			int count1 = 0;	int count2 = 0;

			// b1, b2 are the boundings for first child, b3, b4 for the second one
			Vector3 b1, b2, b3, b4;

			// First swept for first child from 0 to i
			for (int j = 0; j < i; j++) {
				count1 += count[j];
				boundsUnion(b1, b2, b1s[j], b2s[j], b1, b2);
			}

			// Second swept for second child from i to BVH_SAHBINS - 1
			for (int k = i; k < BVH_SAHBINS; k++) {
				count2 += count[k];
				boundsUnion(b3, b4, b1s[k], b2s[k], b3, b4);
			}

			float heuristic = boundsArea(b1, b2) * (float)count1 + boundsArea(b3, b4) * (float)count2;

			if (heuristic < bestHeuristic) {
				bestHeuristic = heuristic;
				bestBin = i;
				bestAxis = axis;
			}
		}
	}
	
	// Tri division depending on the bin
	for (int i = 0; i < tris->size(); i++) {

		float c = tris->at(i).tri.centroid()[bestAxis];
		int bin = map(c, totalB1[bestAxis], totalB2[bestAxis], 0, BVH_SAHBINS - 1);

		if (bin < bestBin) {
			trisLeft->push_back(tris->at(i));
		}
		else {
			trisRight->push_back(tris->at(i));
		}
	}
}

void BVH::divideNaive(std::vector<BVHTri>* tris, std::vector<BVHTri>* trisLeft, std::vector<BVHTri>* trisRight) {

	// Cogemos un triángulo al azar, buscamos el más lejano y los convertimos en centroides de la división


	int furtherIdx = 0;

	for (int i = 0; i < tris->size(); i++) {

		Vector3 c0 = tris->at(0).tri.centroid();
		Vector3 ci = tris->at(i).tri.centroid();
		Vector3 cf = tris->at(furtherIdx).tri.centroid();

		if ((c0 - ci).length() > (c0 - cf).length()){
			furtherIdx = i;
		}	
	}


	for (int i = 0; i < tris->size(); i++) {

		if ((tris->at(0).tri.centroid() - tris->at(i).tri.centroid()).length() < (tris->at(furtherIdx).tri.centroid() - tris->at(i).tri.centroid()).length()) {
			trisLeft->push_back(tris->at(i));
		}
		else {
			trisRight->push_back(tris->at(i));
		}
	}
}

void BVH::boundsUnion(Vector3 b1, Vector3 b2, Vector3 b3, Vector3 b4, Vector3& b5, Vector3& b6) {

	if (boundsArea(b1, b2) <= 0 || boundsArea(b3, b4) <= 0) {

		if (boundsArea(b1, b2) <= 0) {
			b5 = b3;
			b6 = b4;
		}

		if (boundsArea(b3, b4) <= 0) {
			b5 = b1;
			b6 = b2;
		}
	}
	else {

		b5.x = minf(b1.x, minf(b2.x, minf(b3.x, b4.x)));
		b5.y = minf(b1.y, minf(b2.y, minf(b3.y, b4.y)));
		b5.z = minf(b1.z, minf(b2.z, minf(b3.z, b4.z)));

		b6.x = maxf(b1.x, maxf(b2.x, maxf(b3.x, b4.x)));
		b6.y = maxf(b1.y, maxf(b2.y, maxf(b3.y, b4.y)));
		b6.z = maxf(b1.z, maxf(b2.z, maxf(b3.z, b4.z)));
	}

}

float BVH::boundsArea(Vector3 b1, Vector3 b2) {

	float x = b2.x - b1.x;
	float y = b2.y - b1.y;
	float z = b2.z - b1.z;

	return 2 * (x * y + x * z + y * z);

}

void BVH::bounds(Tri tri, Vector3& b1, Vector3& b2) {

	b1.x = minf(tri.vertices[0].x, minf(tri.vertices[1].x, tri.vertices[2].x));
	b1.y = minf(tri.vertices[0].y, minf(tri.vertices[1].y, tri.vertices[2].y));
	b1.z = minf(tri.vertices[0].z, minf(tri.vertices[1].z, tri.vertices[2].z));

	b2.x = maxf(tri.vertices[0].x, maxf(tri.vertices[1].x, tri.vertices[2].x));
	b2.y = maxf(tri.vertices[0].y, maxf(tri.vertices[1].y, tri.vertices[2].y));
	b2.z = maxf(tri.vertices[0].z, maxf(tri.vertices[1].z, tri.vertices[2].z));
}

void BVH::bounds(std::vector<BVHTri>* tris, Vector3& b1, Vector3& b2) {


	if (tris->size() <= 0) {
		//b1 = Vector3();
		//b2 = Vector3();
		return;
	}

	b1 = tris->at(0).tri.vertices[0];
	b2 = tris->at(0).tri.vertices[0];

	for (int i = 0; i < tris->size(); i++) {

		b1.x = minf(tris->at(i).tri.vertices[0].x, b1.x);
		b1.y = minf(tris->at(i).tri.vertices[0].y, b1.y);
		b1.z = minf(tris->at(i).tri.vertices[0].z, b1.z);

		b1.x = minf(tris->at(i).tri.vertices[1].x, b1.x);
		b1.y = minf(tris->at(i).tri.vertices[1].y, b1.y);
		b1.z = minf(tris->at(i).tri.vertices[1].z, b1.z);

		b1.x = minf(tris->at(i).tri.vertices[2].x, b1.x);
		b1.y = minf(tris->at(i).tri.vertices[2].y, b1.y);
		b1.z = minf(tris->at(i).tri.vertices[2].z, b1.z);

		b2.x = maxf(tris->at(i).tri.vertices[0].x, b2.x);
		b2.y = maxf(tris->at(i).tri.vertices[0].y, b2.y);
		b2.z = maxf(tris->at(i).tri.vertices[0].z, b2.z);

		b2.x = maxf(tris->at(i).tri.vertices[1].x, b2.x);
		b2.y = maxf(tris->at(i).tri.vertices[1].y, b2.y);
		b2.z = maxf(tris->at(i).tri.vertices[1].z, b2.z);

		b2.x = maxf(tris->at(i).tri.vertices[2].x, b2.x);
		b2.y = maxf(tris->at(i).tri.vertices[2].y, b2.y);
		b2.z = maxf(tris->at(i).tri.vertices[2].z, b2.z);
	}

}

