#include "raytracer.h"
#include "config.h"
#include <iostream>
#include <chrono>

// External camera and input handling
extern Camera camera;
extern float lastX, lastY, deltaTime, lastFrame;
extern bool firstMouse;
GLFWwindow* startGLFW();
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

Raytracer::Raytracer() 
    : width(SCR_WIDTH), height(SCR_HEIGHT), window(nullptr),
    spheresSSBO(0), sphereDataSSBO(0), sphereData2SSBO(0),
    octreeNodesSSBO(0), octreeNodes2SSBO(0), octreeCountsSSBO(0), objectIndicesSSBO(0),
    raytracingQuad(nullptr), shader(nullptr) {
}

Raytracer::~Raytracer() {
    cleanupBuffers();
    glfwDestroyWindow(window);
    glfwTerminate();
}

bool Raytracer::initialize(){
    window = startGLFW();
    if (!window) return false;
    
    setupQuad();
    setupShader();
    
    return true;
}

void Raytracer::setupQuad(){
  std::vector<float> quadVertices = {
        // positions        // texture coords
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        1.0f,  1.0f, 0.0f, 1.0f, 1.0f
    };

    raytracingQuad = new Mesh(quadVertices, {}, false, false);
    glEnable(GL_DEPTH_TEST);  
}

void Raytracer::setupShader(){
    shader = new Shader("shaders/vertex_shader.glsl", "shaders/octree_fragment_shader.glsl");
}

void Raytracer::setupScene(){
    spheres = generateSpheres();

    int maxDepth = DEBUG ? DEBUGDEPTH : MAXDEPTH;
    int maxSpheresPerNode = DEBUG ? DEBUGSPHERESPERNODE : MAXSPHERESPERNODE;

    octree = Octree(maxDepth, maxSpheresPerNode);
    octree.build(spheres, DEBUG);

    if (DEBUG) octree.printFlattenedTree();
}

void Raytracer::setupBuffers() {
    std::vector<glm::vec4> sphereCentersAndRadii;      // center.xyz, radius
    std::vector<glm::vec4> sphereMaterialsAndAlbedo;   // materialType, albedo.xyz
    std::vector<glm::vec4> sphereFuzzAndRI;           // fuzz, refractionIndex, 0, 0

    std::vector<glm::vec4> octreeMinAndChildren;      // min.xyz, childrenOffset
    std::vector<glm::vec4> octreeMaxAndObjects;       // max.xyz, objectsOffset
    std::vector<int> octreeObjectCounts;              // objectCount

    sphereCentersAndRadii.reserve(spheres.size());
    sphereMaterialsAndAlbedo.reserve(spheres.size());
    sphereFuzzAndRI.reserve(spheres.size());

    for (const Sphere& sphere : spheres) {
        sphereCentersAndRadii.push_back(glm::vec4(sphere.center, sphere.radius));
        sphereMaterialsAndAlbedo.push_back(glm::vec4(float(sphere.materialType), sphere.albedo.x, sphere.albedo.y, sphere.albedo.z));
        sphereFuzzAndRI.push_back(glm::vec4(sphere.fuzz, sphere.refractionIndex, 0.0f, 0.0f));
    }

    octreeMinAndChildren.reserve(octree.flattenedTree.size());
    octreeMaxAndObjects.reserve(octree.flattenedTree.size());
    octreeObjectCounts.reserve(octree.flattenedTree.size());

    for (const GPUOctreeNode& node : octree.flattenedTree) {
        octreeMinAndChildren.push_back(glm::vec4(node.min, node.childrenOffset));
        octreeMaxAndObjects.push_back(glm::vec4(node.max, node.objectsOffset));
        octreeObjectCounts.push_back(node.objectCount);
    }


    glGenBuffers(1, &spheresSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, spheresSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sphereCentersAndRadii.size() * sizeof(glm::vec4), sphereCentersAndRadii.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, spheresSSBO);

    glGenBuffers(1, &sphereDataSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, sphereDataSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sphereMaterialsAndAlbedo.size() * sizeof(glm::vec4), sphereMaterialsAndAlbedo.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, sphereDataSSBO);

    glGenBuffers(1, &sphereData2SSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, sphereData2SSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sphereFuzzAndRI.size() * sizeof(glm::vec4), sphereFuzzAndRI.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, sphereData2SSBO);

    glGenBuffers(1, &octreeNodesSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, octreeNodesSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, octreeMinAndChildren.size() * sizeof(glm::vec4), octreeMinAndChildren.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, octreeNodesSSBO);

    glGenBuffers(1, &octreeNodes2SSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, octreeNodes2SSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, octreeMaxAndObjects.size() * sizeof(glm::vec4), octreeMaxAndObjects.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, octreeNodes2SSBO);

    glGenBuffers(1, &octreeCountsSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, octreeCountsSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, octreeObjectCounts.size() * sizeof(int), octreeObjectCounts.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, octreeCountsSSBO);

    glGenBuffers(1, &objectIndicesSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, objectIndicesSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, octree.objectIndices.size() * sizeof(int), octree.objectIndices.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, objectIndicesSSBO);

    shader->use();
    shader->setInt("useOctree", 1);
    shader->setInt("octreeNodeCount", octree.flattenedTree.size());
    shader->setInt("sphereCount", spheres.size());
    shader->setInt("numSamples", NUMSAMPLES);
    shader->setInt("maxDepth", MAXRAYSDEPTH);

    glm::mat4 projection;
    projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
    shader->setMat4("projection", projection); 

    shader->setVec3("iResolution", SCR_WIDTH, SCR_HEIGHT, 0.0f); 
    shader->setMat4("model", glm::mat4(1.0f));
}

void Raytracer::cleanupBuffers() {
    glDeleteBuffers(1, &spheresSSBO);
    glDeleteBuffers(1, &sphereDataSSBO);
    glDeleteBuffers(1, &sphereData2SSBO);
    glDeleteBuffers(1, &octreeNodesSSBO);
    glDeleteBuffers(1, &octreeNodes2SSBO);
    glDeleteBuffers(1, &octreeCountsSSBO);
    glDeleteBuffers(1, &objectIndicesSSBO);
}

vector<Sphere> Raytracer::generateSpheres() {
    std::vector<Sphere> spheres;

    if (DEBUG) {
        spheres.push_back(Sphere(vec3( -10.000000, -10.000000, -10.000000), 3.000000, 0, vec3( 0.596282, 0.140784, 0.017972), 1.000000, 1.000000)); // (-13, -13, -13) to (-7, -7, -7)
        spheres.push_back(Sphere(vec3( 10.000000, 10.000000, 10.000000), 3.000000, 0,vec3( 0.952200, 0.391551, 0.915972), 1.000000, 1.000000)); // (7, 7, 7) to (13, 13, 13)
        spheres.push_back(Sphere(vec3( -10.000000, 10.000000, -10.000000), 3.000000, 0, vec3( 0.002612, 0.598319, 0.435378), 1.000000, 1.000000)); // (-13, 7, -13) to (-7, 13, -7)
        return spheres;
    }

    spheres.push_back(Sphere(vec3( 0.000000, -1000.000000, 0.000000), 1000.000000, 0, vec3( 0.500000, 0.500000, 0.500000), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( -7.995381, 0.200000, -7.478668), 0.200000, 0, vec3( 0.380012, 0.506085, 0.762437), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( -7.696819, 0.200000, -5.468978), 0.200000, 0, vec3( 0.596282, 0.140784, 0.017972), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( -7.824804, 0.200000, -3.120637), 0.200000, 0, vec3( 0.288507, 0.465652, 0.665070), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( -7.132909, 0.200000, -1.701323), 0.200000, 0, vec3( 0.101047, 0.293493, 0.813446), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( -7.569523, 0.200000, 0.494554), 0.200000, 0, vec3( 0.365924, 0.221622, 0.058332), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( -7.730332, 0.200000, 2.358976), 0.200000, 0, vec3( 0.051231, 0.430547, 0.454086), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( -7.892865, 0.200000, 4.753728), 0.200000, 1, vec3( 0.826684, 0.820511, 0.908836), 0.389611, 1.000000));
    spheres.push_back(Sphere(vec3( -7.656691, 0.200000, 6.888913), 0.200000, 0, vec3( 0.346542, 0.225385, 0.180132), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( -7.217835, 0.200000, 8.203466), 0.200000, 1, vec3( 0.600463, 0.582386, 0.608277), 0.427369, 1.000000));
    spheres.push_back(Sphere(vec3( -5.115232, 0.200000, -7.980404), 0.200000, 0, vec3( 0.256969, 0.138639, 0.080293), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( -5.323222, 0.200000, -5.113037), 0.200000, 0, vec3( 0.193093, 0.510542, 0.613362), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( -5.410681, 0.200000, -3.527741), 0.200000, 0, vec3( 0.352200, 0.191551, 0.115972), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( -5.460670, 0.200000, -1.166543), 0.200000, 0, vec3( 0.029486, 0.249874, 0.077989), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( -5.457659, 0.200000, 0.363870), 0.200000, 0, vec3( 0.395713, 0.762043, 0.108515), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( -5.798715, 0.200000, 2.161684), 0.200000, 2, vec3( 0.000000, 0.000000, 0.000000), 1.000000, 1.500000));
    spheres.push_back(Sphere(vec3( -5.116586, 0.200000, 4.470188), 0.200000, 0, vec3( 0.059444, 0.404603, 0.171767), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( -5.273591, 0.200000, 6.795187), 0.200000, 0, vec3( 0.499454, 0.131330, 0.158348), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( -5.120286, 0.200000, 8.731398), 0.200000, 0, vec3( 0.267365, 0.136024, 0.300483), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( -3.601565, 0.200000, -7.895600), 0.200000, 0, vec3( 0.027752, 0.155209, 0.330428), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( -3.735860, 0.200000, -5.163056), 0.200000, 1, vec3( 0.576768, 0.884712, 0.993335), 0.359385, 1.000000));
    spheres.push_back(Sphere(vec3( -3.481116, 0.200000, -3.794556), 0.200000, 0, vec3( 0.405104, 0.066436, 0.009339), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( -3.866858, 0.200000, -1.465965), 0.200000, 0, vec3( 0.027570, 0.021652, 0.252798), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( -3.168870, 0.200000, 0.553099), 0.200000, 0, vec3( 0.421992, 0.107577, 0.177504), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( -3.428552, 0.200000, 2.627547), 0.200000, 1, vec3( 0.974029, 0.653443, 0.571877), 0.312780, 1.000000));
    spheres.push_back(Sphere(vec3( -3.771736, 0.200000, 4.324785), 0.200000, 0, vec3( 0.685957, 0.000043, 0.181270), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( -3.768522, 0.200000, 6.384588), 0.200000, 0, vec3( 0.025972, 0.082246, 0.138765), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( -3.286992, 0.200000, 8.441148), 0.200000, 0, vec3( 0.186577, 0.560376, 0.367045), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( -1.552127, 0.200000, -7.728200), 0.200000, 0, vec3( 0.202998, 0.002459, 0.015350), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( -1.360796, 0.200000, -5.346098), 0.200000, 0, vec3( 0.690820, 0.028470, 0.179907), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( -1.287209, 0.200000, -3.735321), 0.200000, 0, vec3( 0.345974, 0.672353, 0.450180), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( -1.344859, 0.200000, -1.726654), 0.200000, 0, vec3( 0.209209, 0.431116, 0.164732), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( -1.974774, 0.200000, 0.183260), 0.200000, 0, vec3( 0.006736, 0.675637, 0.622067), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( -1.542872, 0.200000, 2.067868), 0.200000, 0, vec3( 0.192247, 0.016661, 0.010109), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( -1.743856, 0.200000, 4.752810), 0.200000, 0, vec3( 0.295270, 0.108339, 0.276513), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( -1.955621, 0.200000, 6.493702), 0.200000, 0, vec3( 0.270527, 0.270494, 0.202029), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( -1.350449, 0.200000, 8.068503), 0.200000, 1, vec3( 0.646942, 0.501660, 0.573693), 0.346551, 1.000000));
    spheres.push_back(Sphere(vec3( 0.706123, 0.200000, -7.116040), 0.200000, 0, vec3( 0.027695, 0.029917, 0.235781), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( 0.897766, 0.200000, -5.938681), 0.200000, 0, vec3( 0.114934, 0.046258, 0.039647), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( 0.744113, 0.200000, -3.402960), 0.200000, 0, vec3( 0.513631, 0.335578, 0.204787), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( 0.867750, 0.200000, -1.311908), 0.200000, 0, vec3( 0.400246, 0.000956, 0.040513), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( 0.082480, 0.200000, 0.838206), 0.200000, 0, vec3( 0.594141, 0.215068, 0.025718), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( 0.649692, 0.200000, 2.525103), 0.200000, 1, vec3( 0.602157, 0.797249, 0.614694), 0.341860, 1.000000));
    spheres.push_back(Sphere(vec3( 0.378574, 0.200000, 4.055579), 0.200000, 0, vec3( 0.005086, 0.003349, 0.064403), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( 0.425844, 0.200000, 6.098526), 0.200000, 0, vec3( 0.266812, 0.016602, 0.000853), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( 0.261365, 0.200000, 8.661150), 0.200000, 0, vec3( 0.150201, 0.007353, 0.152506), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( 2.814218, 0.200000, -7.751227), 0.200000, 1, vec3( 0.570094, 0.610319, 0.584192), 0.018611, 1.000000));
    spheres.push_back(Sphere(vec3( 2.050073, 0.200000, -5.731364), 0.200000, 0, vec3( 0.109886, 0.029498, 0.303265), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( 2.020130, 0.200000, -3.472627), 0.200000, 0, vec3( 0.216908, 0.216448, 0.221775), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( 2.884277, 0.200000, -1.232662), 0.200000, 0, vec3( 0.483428, 0.027275, 0.113898), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( 2.644454, 0.200000, 0.596324), 0.200000, 0, vec3( 0.005872, 0.860718, 0.561933), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( 2.194283, 0.200000, 2.880603), 0.200000, 0, vec3( 0.452710, 0.824152, 0.045179), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( 2.281000, 0.200000, 4.094307), 0.200000, 0, vec3( 0.002091, 0.145849, 0.032535), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( 2.080841, 0.200000, 6.716384), 0.200000, 0, vec3( 0.468539, 0.032772, 0.018071), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( 2.287131, 0.200000, 8.583242), 0.200000, 2, vec3( 0.000000, 0.000000, 0.000000), 1.000000, 1.500000));
    spheres.push_back(Sphere(vec3( 4.329136, 0.200000, -7.497218), 0.200000, 0, vec3( 0.030865, 0.071452, 0.016051), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( 4.502115, 0.200000, -5.941060), 0.200000, 2, vec3( 0.000000, 0.000000, 0.000000), 1.000000, 1.500000));
    spheres.push_back(Sphere(vec3( 4.750631, 0.200000, -3.836759), 0.200000, 0, vec3( 0.702578, 0.084798, 0.141374), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( 4.082084, 0.200000, -1.180746), 0.200000, 0, vec3( 0.043052, 0.793077, 0.018707), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( 4.429173, 0.200000, 2.069721), 0.200000, 0, vec3( 0.179009, 0.147750, 0.617371), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( 4.277152, 0.200000, 4.297482), 0.200000, 0, vec3( 0.422693, 0.011222, 0.211945), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( 4.012743, 0.200000, 6.225072), 0.200000, 0, vec3( 0.986275, 0.073358, 0.133628), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( 4.047066, 0.200000, 8.419360), 0.200000, 1, vec3( 0.878749, 0.677170, 0.684995), 0.243932, 1.000000));
    spheres.push_back(Sphere(vec3( 6.441846, 0.200000, -7.700798), 0.200000, 0, vec3( 0.309255, 0.342524, 0.489512), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( 6.047810, 0.200000, -5.519369), 0.200000, 0, vec3( 0.532361, 0.008200, 0.077522), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( 6.779211, 0.200000, -3.740542), 0.200000, 0, vec3( 0.161234, 0.539314, 0.016667), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( 6.430776, 0.200000, -1.332107), 0.200000, 0, vec3( 0.641951, 0.661402, 0.326114), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( 6.476387, 0.200000, 0.329973), 0.200000, 0, vec3( 0.033000, 0.648388, 0.166911), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( 6.568686, 0.200000, 2.116949), 0.200000, 0, vec3( 0.590952, 0.072292, 0.125672), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( 6.371189, 0.200000, 4.609841), 0.200000, 1, vec3( 0.870345, 0.753830, 0.933118), 0.233489, 1.000000));
    spheres.push_back(Sphere(vec3( 6.011877, 0.200000, 6.569579), 0.200000, 0, vec3( 0.044868, 0.651697, 0.086779), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( 6.096087, 0.200000, 8.892333), 0.200000, 0, vec3( 0.588587, 0.078723, 0.044928), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( 8.185763, 0.200000, -7.191109), 0.200000, 1, vec3( 0.989702, 0.886784, 0.540759), 0.104229, 1.000000));
    spheres.push_back(Sphere(vec3( 8.411960, 0.200000, -5.285309), 0.200000, 0, vec3( 0.139604, 0.022029, 0.461688), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( 8.047109, 0.200000, -3.427552), 0.200000, 1, vec3( 0.815002, 0.631228, 0.806757), 0.150782, 1.000000));
    spheres.push_back(Sphere(vec3( 8.119639, 0.200000, -1.652587), 0.200000, 0, vec3( 0.177852, 0.429797, 0.042251), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( 8.818120, 0.200000, 0.401292), 0.200000, 0, vec3( 0.065416, 0.087694, 0.040518), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( 8.754155, 0.200000, 2.152549), 0.200000, 0, vec3( 0.230659, 0.035665, 0.435895), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( 8.595298, 0.200000, 4.802001), 0.200000, 0, vec3( 0.188493, 0.184933, 0.040215), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( 8.036216, 0.200000, 6.739752), 0.200000, 0, vec3( 0.023192, 0.364636, 0.464844), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( 8.256561, 0.200000, 8.129115), 0.200000, 0, vec3( 0.002612, 0.598319, 0.435378), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( 0.000000, 1.000000, 0.000000), 1.000000, 2, vec3( 0.000000, 0.000000, 0.000000), 1.000000, 1.500000));
    spheres.push_back(Sphere(vec3( -4.000000, 1.000000, 0.000000), 1.000000, 0, vec3( 0.400000, 0.200000, 0.100000), 1.000000, 1.000000));
    spheres.push_back(Sphere(vec3( 4.000000, 1.000000, 0.000000), 1.000000, 1, vec3( 0.700000, 0.600000, 0.500000), 0.000000, 1.000000));

    return spheres;
}

void Raytracer::run() {
    setupScene();
    setupBuffers();

    // Main render loop
    while (!glfwWindowShouldClose(window)) {
        const auto start2{std::chrono::steady_clock::now()};
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view = camera.GetViewMatrix();

        shader->use();
        shader->setMat4("view", view); 
        shader->setVec3("cameraPosition", camera.Position); 
        shader->setFloat("cameraZoom", camera.Zoom);  

        raytracingQuad->bind();
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
        const auto finish2{std::chrono::steady_clock::now()};
        const std::chrono::duration<double> elapsed_seconds2{finish2 - start2};
        cout << "Main loop time: " << elapsed_seconds2.count() << "s" << std::endl;
    }

    cleanupBuffers();
}