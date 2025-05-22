#ifndef RAYTRACER_H
#define RAYTRACER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "opengl/Shader.h"
#include "opengl/Mesh.h"
#include "opengl/camera.h"
#include "octree.h"
#include "sphere.h"
#include <vector>

class Raytracer {
    public:
        Raytracer();
        ~Raytracer();

        bool initialize();
        void run();
    private:
        // Window and OpenGL
        GLFWwindow* window;
        int width, height;
        
        // Rendering
        Mesh* raytracingQuad;
        Shader* shader;
        
        // Scene
        std::vector<Sphere> spheres;
        Octree octree;
        
        // GPU buffer objects
        GLuint spheresSSBO;
        GLuint sphereDataSSBO;
        GLuint sphereData2SSBO;
        GLuint octreeNodesSSBO;
        GLuint octreeNodes2SSBO;
        GLuint octreeCountsSSBO;
        GLuint objectIndicesSSBO;

        // methods
        void setupQuad();
        void setupShader();
        void setupScene();
        void setupBuffers();
        void cleanupBuffers();
        /**
         * @brief Generate a vector of spheres with predefined properties.
        */
        vector<Sphere> generateSpheres();
        vector<Sphere> generatePreBuiltSpheres();
        vector<Sphere> generateRandomSpheres();
};

#endif // RAYTRACER_H