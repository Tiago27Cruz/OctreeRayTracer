#include <iostream>
#include <glm/glm.hpp>
#include <vector>
#include "sphere.h"
using namespace std;

#define TopLeftFront 0
#define TopRightFront 1
#define BottomRightFront 2
#define BottomLeftFront 3
#define TopLeftBottom 4
#define TopRightBottom 5
#define BottomRightBack 6
#define BottomLeftBack 7

// The same structure as the one in the fragment shader
struct GPUOctreeNode {
    glm::vec3 min;
    glm::vec3 max;
    int childrenOffset;
    int objectsOffset;
    int objectCount;
};

class OctreeNode {
    public:
        OctreeNode* children[8];
        bool isLeaf;
        glm::vec3 min; // Bounding box min corner
        glm::vec3 max; // Bounding box max corner
        std::vector<int> objectIndices; // Indices of objects in this node

        // For flat array GPU representation
        int childrenOffset; // Offset to children in flat array (-1 if leaf)
        int objectsOffset; // Offset to object indices array (-1 if not leaf)
        int objectCount; // Number of objects in this node

        OctreeNode(const glm::vec3& min, const glm::vec3& max);
        ~OctreeNode();
};

class Octree {
    public:
        Octree(int maxDepth = 8, int maxSpheresPerNode = 8);
        ~Octree();

        void build(const vector<Sphere>& spheres);

        vector<GPUOctreeNode> flattenTree();
        vector<int> getObjectIndices();
    private:
        OctreeNode* root;
        int maxDepth;
        int maxSpheresPerNode;

        void subdivideNode(OctreeNode* node, const vector<Sphere>& spheres, int depth);
        bool sphereIntersectsBox(const Sphere& sphere, const glm::vec3& boxMin, const glm::vec3& boxMax);
        
};