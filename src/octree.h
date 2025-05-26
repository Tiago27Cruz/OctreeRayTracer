#ifndef OCTREE_H
#define OCTREE_H

#include <iostream>
#include <glm/glm.hpp>
#include <vector>
#include "sphere.h"
using namespace std;

enum OctantPosition {
    // Binary: zxy (0=min, 1=max for each dimension)
    BottomLeftBack   = 0,  // 000: (min.z, min.x, min.y)
    BottomLeftFront  = 1,  // 001: (min.z, min.x, max.y)
    BottomRightBack  = 2,  // 010: (min.z, max.x, min.y)
    BottomRightFront = 3,  // 011: (min.z, max.x, max.y)
    TopLeftBack      = 4,  // 100: (max.z, min.x, min.y)
    TopLeftFront     = 5,  // 101: (max.z, min.x, max.y)
    TopRightBack     = 6,  // 110: (max.z, max.x, min.y)
    TopRightFront    = 7   // 111: (max.z, max.x, max.y)
};


// The same structure as the one in the fragment shader
struct GPUOctreeNode {
    glm::vec3 min; // Bottom Left Back
    glm::vec3 max; // Top Right Front
    int childrenOffset;
    int objectsOffset;
    int objectCount;
};

class OctreeNode {
    public:
        OctreeNode* children[8];
        bool isLeaf;
        glm::vec3 min; // Bottom Left Back
        glm::vec3 max; // Top Right Front

        int childrenOffset; // Offset to first children in flat array (-1 if leaf)
        // there is no need for childrenCount, as if it has any children, it must have 8 children

        std::vector<int> objectIndices; // Indices of the spheres in this node, i.e. if the sphere1 and sphere3 are in this node, the objectIndices will be [1, 3]
        int objectsOffset; // Offset to object indices array (-1 if not leaf, has it has no objects (spheres))
        int objectCount; // Number of objects in this node

        OctreeNode(const glm::vec3& min, const glm::vec3& max);
        ~OctreeNode();
};

class Octree {
    public:
        Octree(int maxDepth = 8, int maxSpheresPerNode = 8);
        ~Octree();

        // Vectors for GPU
        vector<GPUOctreeNode> flattenedTree;
        vector<int> objectIndices;

        double buildTime = 0.0;

        void build(const vector<Sphere>& spheres, const int debug = 0);

        void setGPUData();

        void printFlattenedTree();
    private:
        OctreeNode* root;
        int maxDepth;
        int maxSpheresPerNode;
        
        // Build functions
        void subdivideNode(OctreeNode* node, const vector<Sphere>& spheres, int depth, const int debug = 0);
        bool sphereIntersectsBox(const Sphere& sphere, const glm::vec3& boxMin, const glm::vec3& boxMax);

        // Cleanup functions
        void cleanup();
        void cleanupNode(OctreeNode* node);
        
};

#endif // OCTREE_H