#include "octree.h"
#include <glm/glm.hpp>

OctreeNode::OctreeNode(const glm::vec3& min, const glm::vec3& max)
    : min(min), max(max), isLeaf(true), childrenOffset(-1), objectsOffset(-1), objectCount(0) {
    for (int i = 0; i < 8; ++i) {
        children[i] = nullptr;
    }
}

OctreeNode::~OctreeNode() {
    for (int i = 0; i < 8; ++i) {
        delete children[i];
    }
}

Octree::Octree(int maxDepth, int maxSpheresPerNode)
    : root(nullptr), maxDepth(maxDepth), maxSpheresPerNode(maxSpheresPerNode) {}

Octree::~Octree() {
    delete root;
}

void Octree::build(const std::vector<Sphere>& spheres) {
    if (spheres.empty()) {
        throw std::invalid_argument("Sphere list is empty");
        return;
    }

    glm::vec3 min = spheres[0].center - glm::vec3(spheres[0].radius);
    glm::vec3 max = spheres[0].center + glm::vec3(spheres[0].radius);

    for (const Sphere& sphere : spheres) {
        glm::vec3 sphereMin = sphere.center - glm::vec3(sphere.radius);
        glm::vec3 sphereMax = sphere.center + glm::vec3(sphere.radius);

        min = glm::min(min, sphereMin);
        max = glm::max(max, sphereMax);
    }
    
    // Root OctreeNode contains everything
    root = new OctreeNode(min, max);

    // Initialize the root node with the bounding box of all spheres
    for (int i = 0; i < spheres.size(); ++i) {
        root->objectIndices.push_back(i);
    }
    root->objectCount = spheres.size();
}

void Octree::subdivideNode(OctreeNode* node, const std::vector<Sphere>& spheres, int depth) {
    // Stop if we're at max depth or we have few enough spheres
    if (depth >= maxDepth || node->objectIndices.size() <= static_cast<size_t>(maxSpheresPerNode)) {
        return;
    }
    
    // Calculate midpoint of current node
    glm::vec3 mid = (node->min + node->max) * 0.5f;
    
    // Mark node as internal (not a leaf)
    node->isLeaf = false;
    
    // Create 8 children
    for (int i = 0; i < 8; ++i) {
        // Calculate child bounds based on position
        glm::vec3 childMin = node->min;
        glm::vec3 childMax = node->max;
        
        // Adjust bounds based on octant position
        if (i & 1) childMin.x = mid.x; else childMax.x = mid.x;
        if (i & 2) childMin.y = mid.y; else childMax.y = mid.y;
        if (i & 4) childMin.z = mid.z; else childMax.z = mid.z;
        
        node->children[i] = new OctreeNode(childMin, childMax);
    }
    
    // Distribute spheres to child nodes
    for (int sphereIdx : node->objectIndices) {
        const Sphere& sphere = spheres[sphereIdx];
        
        // Add sphere to each intersecting child
        for (int i = 0; i < 8; ++i) {
            if (sphereIntersectsBox(sphere, node->children[i]->min, node->children[i]->max)) {
                node->children[i]->objectIndices.push_back(sphereIdx);
                node->children[i]->objectCount++;
            }
        }
    }
    
    // Clear parent's object list to save memory
    node->objectIndices.clear();
    
    // Recursively subdivide children
    for (int i = 0; i < 8; ++i) {
        if (!node->children[i]->objectIndices.empty()) {
            subdivideNode(node->children[i], spheres, depth + 1);
        }
    }
}

bool Octree::sphereIntersectsBox(const Sphere& sphere, const glm::vec3& boxMin, const glm::vec3& boxMax) {
    // Calculate closest point on box to sphere center
    glm::vec3 closest;
    
    // For each axis, find closest point on box boundary to sphere center
    for (int i = 0; i < 3; ++i) {
        closest[i] = std::max(boxMin[i], std::min(sphere.center[i], boxMax[i]));
    }
    
    // Check if closest point is within sphere radius
    float distSquared = glm::length(closest - sphere.center);
    return distSquared <= (sphere.radius * sphere.radius);
}

