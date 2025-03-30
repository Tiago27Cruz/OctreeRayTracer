#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <glad/glad.h>

class Shader {
public:
    unsigned int ID;

    // Constructor reads and builds the shader
    Shader(const char* vertexPath, const char* fragmentPath);

    // Use/activate the shader
    void use();

    // Utility functions to set uniforms
    void setBool(const std::string &name, bool value) const;
    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string &name, float value) const;
    void deleteProgram() const { glDeleteProgram(ID); } 

private:
    void checkCompileErrors(unsigned int shader, const std::string& type);
};

#endif