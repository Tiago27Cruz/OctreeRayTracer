cmake_minimum_required(VERSION 3.10.0)
project(edaa VERSION 0.1.0)


# windows config
if (WIN32)
    include_directories(include)

    set(GLFW_LIB_PATH "${CMAKE_SOURCE_DIR}/lib")

    add_executable(edaa src/main.cpp src/glad.c src/opengl/shaders.cpp src/opengl/shader.h src/opengl/shader.cpp src/opengl/mesh.cpp src/opengl/mesh.h src/opengl/camera.h)

    target_link_directories(edaa PRIVATE "${GLFW_LIB_PATH}")
    target_link_libraries(edaa "${GLFW_LIB_PATH}\\libglfw3.a" opengl32)
    file(COPY shaders DESTINATION ${CMAKE_BINARY_DIR})
else()
    message(FATAL_ERROR "TODO: linux config")
endif()