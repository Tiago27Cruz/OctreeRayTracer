#ifndef MESH_H
#define MESH_H

#include <vector>
#include <glad/glad.h>

class Mesh {
public:
    unsigned int VAO;

    // Constructor: takes vertex data and sets up the buffers
    Mesh(const std::vector<float>& vertices, const std::vector<unsigned int>& indices, bool isIndexed, bool wireframe);

    // Bind the VAO
    void bind() const;

    // Unbind the VAO
    void unbind() const;

    // Destructor: cleans up resources
    ~Mesh();

private:
    unsigned int VBO;
    unsigned int EBO;
};

#endif