#include <glm/glm.hpp>
using namespace glm;

class Sphere {
    public: 
        // sphere properties
        vec3 center;
        float radius;

        // material
        int   materialType;
        vec3  albedo;
        float fuzz;
        float refractionIndex;

        // Constructor
        Sphere(const vec3& center, float radius, int materialType = 0, const vec3& albedo = vec3(1.0f), float fuzz = 0.0f, float refractionIndex = 1.0f)
            : center(center), radius(radius), albedo(albedo), materialType(materialType), fuzz(fuzz), refractionIndex(refractionIndex) {}

};