cmake_minimum_required(VERSION 3.10.0)
project(edaa VERSION 0.1.0)


# windows config
if (WIN32)
    include_directories(include)

    set(GLFW_LIB_PATH "$ENV{USERPROFILE}\\Documents\\GitHub\\EDAA\\lib")

    add_executable(edaa src/main.cpp src/glad.c src/shaders.cpp)

    target_link_directories(edaa PRIVATE "${GLFW_LIB_PATH}")
    target_link_libraries(edaa "${GLFW_LIB_PATH}\\libglfw3.a" opengl32)
else()
    message(FATAL_ERROR "TODO: linux config")
endif()