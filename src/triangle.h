#include <glm/glm.hpp>
using namespace glm;

class Triangle {
public:
    vec3 v0, v1, v2;
    vec3 normal;
    vec3 color;

    Triangle(const vec3& v0, const vec3& v1, const vec3& v2, const vec3& color = vec3(1.0f));
    
    bool intersect(const vec3& origin, const vec3& direction, float& t, vec3& hitNormal) const;
    vec3 getCenter() const;
    bool isInBox(const vec3& min, const vec3& max) const;
};