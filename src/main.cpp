#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "opengl/Shader.h"
#include "opengl/Mesh.h"
#include "opengl/camera.h"
#include "opengl/Sphere.h"
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"


// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

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
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    } 

    // set up vertex data (and buffer(s)) and configure vertex attributes
    std::vector<Sphere> spheres = {
        Sphere(1000.000000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(0.200000f),
        Sphere(1.000000f),
        Sphere(1.000000f),
        Sphere(1.000000f)
    };

    glm::vec3 spherePosition[] = {
        glm::vec3( 0.000000f, -1000.000000f, 0.000000f),
        glm::vec3( -7.995381f, 0.200000f, -7.478668f),
        glm::vec3( -7.696819f, 0.200000f, -5.468978f),
        glm::vec3( -7.824804f, 0.200000f, -3.120637f),
        glm::vec3( -7.132909f, 0.200000f, -1.701323f),
        glm::vec3( -7.569523f, 0.200000f, 0.494554f),
        glm::vec3( -7.730332f, 0.200000f, 2.358976f),
        glm::vec3( -7.892865f, 0.200000f, 4.753728f),
        glm::vec3( -7.656691f, 0.200000f, 6.888913f),
        glm::vec3( -7.217835f, 0.200000f, 8.203466f),
        glm::vec3( -5.115232f, 0.200000f, -7.980404f),
        glm::vec3( -5.323222f, 0.200000f, -5.113037f),
        glm::vec3( -5.410681f, 0.200000f, -3.527741f),
        glm::vec3( -5.460670f, 0.200000f, -1.166543f),
        glm::vec3( -5.457659f, 0.200000f, 0.363870f),
        glm::vec3( -5.798715f, 0.200000f, 2.161684f),
        glm::vec3( -5.116586f, 0.200000f, 4.470188f),
        glm::vec3( -5.273591f, 0.200000f, 6.795187f),
        glm::vec3( -5.120286f, 0.200000f, 8.731398f),
        glm::vec3( -3.601565f, 0.200000f, -7.895600f),
        glm::vec3( -3.735860f, 0.200000f, -5.163056f),
        glm::vec3( -3.481116f, 0.200000f, -3.794556f),
        glm::vec3( -3.866858f, 0.200000f, -1.465965f),
        glm::vec3( -3.168870f, 0.200000f, 0.553099f),
        glm::vec3( -3.428552f, 0.200000f, 2.627547f),
        glm::vec3( -3.771736f, 0.200000f, 4.324785f),
        glm::vec3( -3.768522f, 0.200000f, 6.384588f),
        glm::vec3( -3.286992f, 0.200000f, 8.441148f),
        glm::vec3( -1.552127f, 0.200000f, -7.728200f),
        glm::vec3( -1.360796f, 0.200000f, -5.346098f),
        glm::vec3( -1.287209f, 0.200000f, -3.735321f),
        glm::vec3( -1.344859f, 0.200000f, -1.726654f),
        glm::vec3( -1.974774f, 0.200000f, 0.183260f),
        glm::vec3( -1.542872f, 0.200000f, 2.067868f),
        glm::vec3( -1.743856f, 0.200000f, 4.752810f),
        glm::vec3( -1.955621f, 0.200000f, 6.493702f),
        glm::vec3( -1.350449f, 0.200000f, 8.068503f),
        glm::vec3( 0.706123f, 0.200000f, -7.116040f),
        glm::vec3( 0.897766f, 0.200000f, -5.938681f),
        glm::vec3( 0.744113f, 0.200000f, -3.402960f),
        glm::vec3( 0.867750f, 0.200000f, -1.311908f),
        glm::vec3( 0.082480f, 0.200000f, 0.838206f),
        glm::vec3( 0.649692f, 0.200000f, 2.525103f),
        glm::vec3( 0.378574f, 0.200000f, 4.055579f),
        glm::vec3( 0.425844f, 0.200000f, 6.098526f),
        glm::vec3( 0.261365f, 0.200000f, 8.661150f),
        glm::vec3( 2.814218f, 0.200000f, -7.751227f),
        glm::vec3( 2.050073f, 0.200000f, -5.731364f),
        glm::vec3( 2.020130f, 0.200000f, -3.472627f),
        glm::vec3( 2.884277f, 0.200000f, -1.232662f),
        glm::vec3( 2.644454f, 0.200000f, 0.596324f),
        glm::vec3( 2.194283f, 0.200000f, 2.880603f),
        glm::vec3( 2.281000f, 0.200000f, 4.094307f),
        glm::vec3( 2.080841f, 0.200000f, 6.716384f),
        glm::vec3( 2.287131f, 0.200000f, 8.583242f),
        glm::vec3( 4.329136f, 0.200000f, -7.497218f),
        glm::vec3( 4.502115f, 0.200000f, -5.941060f),
        glm::vec3( 4.750631f, 0.200000f, -3.836759f),
        glm::vec3( 4.082084f, 0.200000f, -1.180746f),
        glm::vec3( 4.429173f, 0.200000f, 2.069721f),
        glm::vec3( 4.277152f, 0.200000f, 4.297482f),
        glm::vec3( 4.012743f, 0.200000f, 6.225072f),
        glm::vec3( 4.047066f, 0.200000f, 8.419360f),
        glm::vec3( 6.441846f, 0.200000f, -7.700798f),
        glm::vec3( 6.047810f, 0.200000f, -5.519369f),
        glm::vec3( 6.779211f, 0.200000f, -3.740542f),
        glm::vec3( 6.430776f, 0.200000f, -1.332107f),
        glm::vec3( 6.476387f, 0.200000f, 0.329973f),
        glm::vec3( 6.568686f, 0.200000f, 2.116949f),
        glm::vec3( 6.371189f, 0.200000f, 4.609841f),
        glm::vec3( 6.011877f, 0.200000f, 6.569579f),
        glm::vec3( 6.096087f, 0.200000f, 8.892333f),
        glm::vec3( 8.185763f, 0.200000f, -7.191109f),
        glm::vec3( 8.411960f, 0.200000f, -5.285309f),
        glm::vec3( 8.047109f, 0.200000f, -3.427552f),
        glm::vec3( 8.119639f, 0.200000f, -1.652587f),
        glm::vec3( 8.818120f, 0.200000f, 0.401292f),
        glm::vec3( 8.754155f, 0.200000f, 2.152549f),
        glm::vec3( 8.595298f, 0.200000f, 4.802001f),
        glm::vec3( 8.036216f, 0.200000f, 6.739752f),
        glm::vec3( 8.256561f, 0.200000f, 8.129115f),
        glm::vec3( 0.000000f, 1.000000f, 0.000000f),
        glm::vec3( -4.000000f, 1.000000f, 0.000000f),
        glm::vec3( 4.000000f, 1.000000f, 0.000000f)
    };

    glEnable(GL_DEPTH_TEST);

    // Build and compile shaders
    Shader shader("shaders/vertex_shader.glsl", "shaders/fragment_shader.glsl");

    /*// Texture
    unsigned int texture1;
    // texture 1
    // ---------
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1); 
     // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
   
    unsigned char *data = stbi_load("../resources/textures/container.jpg", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
    */

    shader.use();
    //shader.setInt("texture1", 0); // Set the texture uniform to 0 (GL_TEXTURE0)
    glm::mat4 projection;
    projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
    shader.setMat4("projection", projection); // Set the projection matrix uniform

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Process input
        processInput(window);

        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // bind textures on corresponding texture units
        //glActiveTexture(GL_TEXTURE0);
        //glBindTexture(GL_TEXTURE_2D, texture1);

        glm::mat4 view = camera.GetViewMatrix();

        // Shaders
        //shader.use();
        shader.setMat4("view", view); // Set the view matrix uniform
        shader.setVec3("cameraPosition", camera.Position);  // Pass camera position
        shader.setFloat("cameraZoom", camera.Zoom);  
        shader.setVec3("iResolution", SCR_WIDTH, SCR_HEIGHT, 0.0f); // Set the resolution uniform
        shader.setFloat("iTime", glfwGetTime()); // Set the time uniform
        shader.setFloat("iTimeDelta", deltaTime); // Set the time delta uniform
        shader.setFloat("iFrameRate", 1.0f / deltaTime); // Set the frame rate uniform
        shader.setInt("iFrame", static_cast<int>(currentFrame)); // Set the frame uniform
        shader.setFloat("iChannelTime[0]", glfwGetTime()); // Set the channel time uniform
        shader.setVec3("iChannelResolution[0]", SCR_WIDTH, SCR_HEIGHT, 0.0f); // Set the channel resolution uniform
        shader.setVec4("iMouse", 0.0f, 0.0f, 0.0f, 0.0f); // Set the mouse uniform
        shader.setVec4("iDate", 0.0f, 0.0f, 0.0f, 0.0f); // Set the date uniform
        shader.setFloat("iSampleRate", 44100.0f); // Set the sample rate uniform

        //raytracingQuad.bind();
        //glDrawArrays(GL_TRIANGLES, 0, 6);

        /*
        for (unsigned int i = 0; i < spheres.size(); i++)
        {
           // glm::mat4 model = glm::mat4(1.0f);
            //model = glm::translate(model, spherePosition[i]);
            //shader.setMat4("model", model);
            spheres[i].draw();
            std::cout<<"draw sphere "<<i<<std::endl;
        }*/
        //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        Sphere sphere1 = Sphere(1.0f);
        sphere1.draw();
        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

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
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}
