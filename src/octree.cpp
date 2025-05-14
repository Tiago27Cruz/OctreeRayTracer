#include "octree.h"
#include <glm/glm.hpp>
#include <map>

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

    glm::vec3 min = spheres[0].center - glm::vec3(spheres[0].radius, spheres[0].radius, spheres[0].radius);
    glm::vec3 max = spheres[0].center + glm::vec3(spheres[0].radius, spheres[0].radius, spheres[0].radius);

    for (const Sphere& sphere : spheres) {
        glm::vec3 sphereMin = sphere.center - glm::vec3(sphere.radius, sphere.radius, sphere.radius);
        glm::vec3 sphereMax = sphere.center + glm::vec3(sphere.radius, sphere.radius, sphere.radius);

        min = glm::min(min, sphereMin);
        max = glm::max(max, sphereMax);
    }
    
    root = new OctreeNode(min, max);

    // since it contains everything, we add all indices to the root node
    for (int i = 0; i < spheres.size(); ++i) {
        root->objectIndices.push_back(i); 
    }
    root->objectCount = spheres.size();

    subdivideNode(root, spheres, 0);
}

void Octree::subdivideNode(OctreeNode* node, const std::vector<Sphere>& spheres, int depth) {
    // Stop if we're at max depth
    if (depth >= maxDepth || node->objectIndices.size() <= static_cast<size_t>(maxSpheresPerNode)) {
        std::cout << "Stopping subdivision at depth " << depth << " with " << node->objectIndices.size() << " objects." << std::endl;
        return;
    }


    glm::vec3 mid = (node->min + node->max) * 0.5f;
    std::cout << "Midpoint: " << mid.x << ", " << mid.y << ", " << mid.z << std::endl;
    

    node->isLeaf = false;
    for (int i = 0; i < 8; ++i) {

        glm::vec3 childMin = node->min;
        glm::vec3 childMax = node->max;
        
        switch(i) {
            case 0: // bottom-left-front
                childMax = mid;
                break;
            case 1: // bottom-right-front
                childMin.x = mid.x;
                childMax.y = mid.y;
                childMax.z = mid.z;
                break;
            case 2: // top-left-front
                childMax.x = mid.x;
                childMin.y = mid.y;
                childMax.z = mid.z;
                break;
            case 3: // top-right-front
                childMin.x = mid.x;
                childMin.y = mid.y;
                childMax.z = mid.z;
                break;
            case 4: // bottom-left-back
                childMax.x = mid.x;
                childMax.y = mid.y;
                childMin.z = mid.z;
                break;
            case 5: // bottom-right-back
                childMin.x = mid.x;
                childMax.y = mid.y;
                childMin.z = mid.z;
                break;
            case 6: // top-left-back
                childMax.x = mid.x;
                childMin.y = mid.y;
                childMin.z = mid.z;
                break;
            case 7: // top-right-back
                childMin = mid;
                break;
        }
        
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
    
    // clear parent's indices to save space
    node->objectIndices.clear();
    node->objectCount = 0;
    

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

/**
 * @brief Flattens the octree into a vector of GPUOctreeNode structures.
 * This function traverses the octree and stores the min, max, childrenOffset,
 * objectsOffset, and objectCount for each node in a flat array format so that
 * it can be passed and used in the fragment shader.
 * 
 * Works like a DFS
 */
vector<GPUOctreeNode> Octree::flattenTree() {
    std::vector<GPUOctreeNode> flattenedNodes;
    std::vector<OctreeNode*> stack;
    std::map<OctreeNode*, int> nodeToIndex;
    std::vector<OctreeNode*> allNodes;

    // Add all nodes to the flattened array
    stack.push_back(root);
    while (!stack.empty()) {
        OctreeNode* node = stack.back();
        stack.pop_back();


        allNodes.push_back(node);
        nodeToIndex[node] = flattenedNodes.size();


        GPUOctreeNode gpuNode;
        gpuNode.min = node->min;
        gpuNode.max = node->max;
        gpuNode.childrenOffset = -1;
        gpuNode.objectsOffset = node->objectsOffset;
        gpuNode.objectCount = node->objectCount;
        flattenedNodes.push_back(gpuNode);

        // Add children to the stack
        if (!node->isLeaf) {
            for (int i = 0; i < 8; ++i) {
                if (node->children[i]) {
                    stack.push_back(node->children[i]);
                }
            }
        }
    }

    // set children offsets
    for (size_t i = 0; i < flattenedNodes.size(); i++) {
        OctreeNode* node = allNodes[i]; 
        if (!node->isLeaf) {
            flattenedNodes[i].childrenOffset = nodeToIndex[node->children[0]];
        }
    }

    return flattenedNodes;
}

// TODO: Maybe could be joined with flattenTree
vector<int> Octree::getObjectIndices() {
    std::vector<int> objectIndices;
    std::vector<OctreeNode*> nodeStack;
    nodeStack.push_back(root);
    
    // First: collect all nodes in the order they'll be processed
    std::vector<OctreeNode*> allNodes;
    while (!nodeStack.empty()) {
        OctreeNode* node = nodeStack.back();
        nodeStack.pop_back();
        
        allNodes.push_back(node);
        
        if (!node->isLeaf) {
            for (int i = 0; i < 8; ++i) {
                if (node->children[i]) {
                    nodeStack.push_back(node->children[i]);
                }
            }
        }
    }
    
    // Second: set offsets and collect indices
    int currentOffset = 0;
    for (OctreeNode* node : allNodes) {
        if (node->isLeaf && !node->objectIndices.empty()) {
            node->objectsOffset = currentOffset;
            objectIndices.insert(objectIndices.end(), node->objectIndices.begin(), node->objectIndices.end()); // adds all indices to the end of the vector
            currentOffset += node->objectIndices.size();
        }
    }
    
    return objectIndices;
}

