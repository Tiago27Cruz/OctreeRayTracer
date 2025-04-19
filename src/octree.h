#include <iostream>
#include <glm/glm.hpp>
#include <vector>
using namespace std;

#define TopLeftFront 0
#define TopRightFront 1
#define BottomRightFront 2
#define BottomLeftFront 3
#define TopLeftBottom 4
#define TopRightBottom 5
#define BottomRightBack 6
#define BottomLeftBack 7

class Triangle;

class OctreeNode {
    public:
        OctreeNode* children[8];
        std::vector<Triangle*> triangles;
        bool isLeaf;

        OctreeNode();
        ~OctreeNode();
};

class Octree {
    public:
        Octree();
        ~Octree();
    private:
        OctreeNode* root;
        int maxDepth;
        int maxTrianglesPerNode;
};