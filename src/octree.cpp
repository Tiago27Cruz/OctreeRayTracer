#include "octree.h"
#include <glm/glm.hpp>
#include <map>
#include <queue>

OctreeNode::OctreeNode(const glm::vec3& min, const glm::vec3& max)
    : min(min), max(max), isLeaf(true), childrenOffset(-1), objectsOffset(-1), objectCount(0){
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
    cleanup();
}

void Octree::cleanup() {
    if (root) {
        cleanupNode(root);
        delete root;
        root = nullptr;
    }
}

void Octree::cleanupNode(OctreeNode* node) {
    if (!node->isLeaf) {
        for (int i = 0; i < 8; ++i) {
            if (node->children[i]) {
                cleanupNode(node->children[i]);
                delete node->children[i];
                node->children[i] = nullptr;
            }
        }
    }
}

void Octree::build(const std::vector<Sphere>& spheres) {
    if (spheres.empty()) {
        throw std::invalid_argument("Sphere list is empty");
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

    // DEBUG
    cout << "Root Node Min: " << root->min.x << ", " << root->min.y << ", " << root->min.z << std::endl; // Min: (-13, -13, -13)
    cout << "Root Node Max: " << root->max.x << ", " << root->max.y << ", " << root->max.z << std::endl; // Max: (13, 13, 13)

    // since it contains everything, we add all indices to the root node
    for (int i = 0; i < spheres.size(); ++i) {
        root->objectIndices.push_back(i); 
    }
    root->objectCount = spheres.size();
    cout << "Root Node Object Count: " << root->objectCount << std::endl; // DEBUG: Object Count: 3

    subdivideNode(root, spheres, 0);

    setGPUData();
}

OctreeNode* createSubnodes(int index, OctreeNode* node, const glm::vec3& mid) {
    glm::vec3 childMin, childMax;
        
    switch (index){
        case TopLeftFront: { // example: (-1, 0, 0) to (0, 1, 1) 
            childMin.x = node->min.x;
            childMin.y = mid.y;
            childMin.z = mid.z;
            
            childMax.x = mid.x;
            childMax.y = node->max.y;
            childMax.z = node->max.z;
            break;
        }
        case TopRightFront: { // example: (0, 0, 0) to (1, 1, 1)
            childMin.x = mid.x;
            childMin.y = mid.y;
            childMin.z = mid.z;
            
            childMax.x = node->max.x;
            childMax.y = node->max.y;
            childMax.z = node->max.z;
            break;
        }
        case BottomRightFront: { // example: (0, 0, -1) to (1, 1, 0)
            childMin.x = mid.x;
            childMin.y = mid.y;
            childMin.z = node->min.z;

            childMax.x = node->max.x;
            childMax.y = node->max.y;
            childMax.z = mid.z;
            break;
        }
        case BottomLeftFront: { // example: (-1, 0, -1) to (0, 1, 0)
            childMin.x = node->min.x;
            childMin.y = mid.y;
            childMin.z = node->min.z;

            childMax.x = mid.x;
            childMax.y = node->max.y;
            childMax.z = mid.z;
            break;
        }
        case TopLeftBack: { // example: (-1, -1, 0) to (0, 0, 1)
            childMin.x = node->min.x;
            childMin.y = node->min.y;
            childMin.z = mid.z;

            childMax.x = mid.x;
            childMax.y = mid.y;
            childMax.z = node->max.z;
            break;
        }
        case TopRightBack: { // example: (0, -1, 0) to (1, 0, 1)
            childMin.x = mid.x;
            childMin.y = node->min.y;
            childMin.z = mid.z;

            childMax.x = node->max.x;
            childMax.y = mid.y;
            childMax.z = node->max.z;
            break;
        }
        case BottomRightBack: { // example: (0, -1, -1) to (1, 0, 0)
            childMin.x = mid.x;
            childMin.y = node->min.y;
            childMin.z = node->min.z;

            childMax.x = node->max.x;
            childMax.y = mid.y;
            childMax.z = mid.z;
            break;
        }
        case BottomLeftBack: { // example: (-1, -1, -1) to (0, 0, 0)
            childMin.x = node->min.x;
            childMin.y = node->min.y;
            childMin.z = node->min.z;

            childMax.x = mid.x;
            childMax.y = mid.y;
            childMax.z = mid.z;
            break;
        }
        default:
            throw std::invalid_argument("Invalid octant index while subdividing node with index = " + std::to_string(index));
            break;
    }

    return new OctreeNode(childMin, childMax);
}

void Octree::subdivideNode(OctreeNode* node, const std::vector<Sphere>& spheres, int depth) {
    // Stop if we're at max depth
    if (depth >= maxDepth || node->objectIndices.size() <= static_cast<size_t>(maxSpheresPerNode)) {
        //std::cout << "Stopping subdivision at depth " << depth << " with " << node->objectIndices.size() << " objects." << std::endl;
        return;
    }


    glm::vec3 mid = (node->min + node->max) * 0.5f;
    std::cout << "Midpoint: " << mid.x << ", " << mid.y << ", " << mid.z << std::endl;
    

    node->isLeaf = false;
    for (int i = 0; i < 8; ++i) {
        node->children[i] = createSubnodes(i, node, mid);
    }
    
    // Distribute spheres to child nodes
    for (int sphereIdx : node->objectIndices) {
        const Sphere& sphere = spheres[sphereIdx];
        
        for (int i = 0; i < 8; ++i) {
            if (sphereIntersectsBox(sphere, node->children[i]->min, node->children[i]->max)) {
                cout << "Sphere " << sphereIdx << " intersects child node " << i << std::endl;
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

// TODO: Understand this lol
bool Octree::sphereIntersectsBox(const Sphere& sphere, const glm::vec3& boxMin, const glm::vec3& boxMax) {
    glm::vec3 closest;
    
    // For each axis, find closest point on box boundary to sphere center
    for (int i = 0; i < 3; ++i) {
        closest[i] = std::max(boxMin[i], std::min(sphere.center[i], boxMax[i]));
    }
    
    // Check if closest point is within sphere radius
    float distSquared = glm::dot(closest - sphere.center, closest - sphere.center);
    return distSquared <= (sphere.radius * sphere.radius);
}

void Octree::printFlattenedTree() {
    int i = 0;
    std::cout << std::endl;
    for (const GPUOctreeNode& node : flattenedTree) {
        std::cout << "Node " << i++ << ":" << std::endl;
        std::cout << "Node Min: " << node.min.x << ", " << node.min.y << ", " << node.min.z << std::endl;
        std::cout << "Node Max: " << node.max.x << ", " << node.max.y << ", " << node.max.z << std::endl;
        std::cout << "Children Offset: " << node.childrenOffset << std::endl;
        std::cout << "Objects Offset: " << node.objectsOffset << std::endl;
        std::cout << "Object Count: " << node.objectCount << std::endl;
        if (node.objectCount > 0) {
            std::cout << "Object Indices: ";
            for (int index : objectIndices) {
                std::cout << index << " ";
            }
            std::cout << std::endl;
        }
        
        std::cout << std::endl;
    }
}
/**
 * BFS type shit
 */
void Octree::setGPUData() {
    std::queue<OctreeNode*> nodeQueue;
    nodeQueue.push(root);

    int currentOffset = 0;
    std::vector<OctreeNode*> allNodes;
    std::map<OctreeNode*, int> nodeToIndex;

    while (!nodeQueue.empty()) {
        OctreeNode* node = nodeQueue.front();
        nodeQueue.pop();

        allNodes.push_back(node);
        nodeToIndex[node] = currentOffset;
        currentOffset++;

        if (!node->isLeaf) {
            for (int i = 0; i < 8; ++i) {
                if (!node->children[i]) throw std::invalid_argument("Child node is null on non-leaf node");
                nodeQueue.push(node->children[i]);
            }
        }
    }

    int currentObjectIndex = 0;
    for (OctreeNode* node : allNodes) {
        if (!node->isLeaf) {
            node->childrenOffset = nodeToIndex[node->children[0]];
        } else if (node->isLeaf && node->objectCount > 0) {
            objectIndices.insert(objectIndices.end(), node->objectIndices.begin(), node->objectIndices.end());
            node->objectsOffset = currentObjectIndex;
            currentObjectIndex += node->objectCount;
        }
    }

    // Flatten the tree
    flattenedTree.reserve(allNodes.size());
    for (OctreeNode* node : allNodes) {
        GPUOctreeNode gpuNode;
        gpuNode.min = node->min;
        gpuNode.max = node->max;
        gpuNode.childrenOffset = node->childrenOffset;
        gpuNode.objectsOffset = node->objectsOffset;
        gpuNode.objectCount = node->objectCount;

        flattenedTree.push_back(gpuNode);
    }
}