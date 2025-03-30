#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "Shader.h"
#include "Mesh.h"

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);  
void processInput(GLFWwindow *window);


int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Set GLFW window hints for OpenGL version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Set GLFW window hints for compatibility with macOS
    #ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    // Create a GLFW window
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "EDAA Window", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Make the OpenGL context current
    glfwMakeContextCurrent(window);
    // Set the viewport size
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);  

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    } 

    // set up vertex data (and buffer(s)) and configure vertex attributes
    std::vector<float> vertices = {
        -0.5f, -0.5f, 0.0f, // left  
         0.5f, -0.5f, 0.0f, // right 
         0.0f,  0.5f, 0.0f  // top   
    };

    // Create a Mesh object
    Mesh triangle(vertices);

    // Build and compile shaders
    Shader shader("shaders/vertex_shader.glsl", "shaders/fragment_shader.glsl");

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // Process input
        processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw the triangle
        shader.use();
        triangle.bind();
        glDrawArrays(GL_TRIANGLES, 0, 3);
        triangle.unbind();

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    triangle.unbind();
    shader.deleteProgram();

    // Clean up and exit
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

/**
 * @brief Callback function to adjust the viewport when the window is resized.
 * @param window Pointer to the GLFW window.
 * @param width New width of the window.
 * @param height New height of the window.
 */
void framebuffer_size_callback(GLFWwindow* window, int width, int height){
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

/**
 * @brief Process input from the user.
 * @param window Pointer to the GLFW window.
 */
void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}