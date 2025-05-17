#version 430 core

#define MAXFLOAT 3.402823466e+38
#define PI 3.14159265359
#define LAMBERT 0
#define METAL 1
#define DIELECTRIC 2

out vec4 FragColor;
in vec2 FragCoord;

// Camera uniforms 
uniform mat4 view;
uniform vec3 cameraPosition;
uniform float cameraZoom;

// Resolution and time
uniform vec3 iResolution;
uniform float iTime;

layout(std430, binding = 0) buffer SphereBuffer {
    vec4 spheres[];
};

layout(std430, binding = 1) buffer SphereDataBuffer {
    vec4 sphereData[];
};

layout(std430, binding = 2) buffer SphereData2Buffer {
    vec4 sphereData2[];
};

layout(std430, binding = 3) buffer OctreeNodeBuffer {
    vec4 octreeNodes[];
};

layout(std430, binding = 4) buffer OctreeNode2Buffer {
    vec4 octreeNodes2[];
};

layout(std430, binding = 5) buffer OctreeCountsBuffer {
    int octreeNodeCounts[];
};

layout(std430, binding = 6) buffer ObjectIndicesBuffer {
    int objectIndices[];
};

uniform int useOctree;
uniform int octreeNodeCount;
uniform int sphereCount;

uniform int numSamples;
uniform int maxDepth;

// Random state for sampling
vec2 randState;

// Ray structure
struct Ray {
    vec3 origin;
    vec3 direction;
};

struct IntersectInfo
{
    // surface properties
    float t;
    vec3  point;
    vec3  normal;
	
    // material properties
    int   materialType;
    vec3  albedo;
    float fuzz;
    float refractionIndex;
};

// Camera structure
struct Camera {
    vec3 origin;
    vec3 lowerLeftCorner;
    vec3 horizontal;
    vec3 vertical;
    vec3 u, v, w;
    float lensRadius;
};

// Random functions
float rand2D() {
    uint state = uint(randState.x * 747796405u + randState.y * 2891336453u);
    state = ((state >> uint(16)) ^ state) * uint(73244475);
    state = ((state >> uint(16)) ^ state) * uint(73244475);
    state = (state >> uint(16)) ^ state;
    
    randState.x = randState.y;
    randState.y = float(state) / 4294967296.0;
    return randState.y;
}


vec3 random_in_unit_disk()
{
    float spx = 2.0 * rand2D() - 1.0;
    float spy = 2.0 * rand2D() - 1.0;

    float r, phi;


    if(spx > -spy)
    {
        if(spx > spy)
        {
            r = spx;
            phi = spy / spx;
        }
        else
        {
            r = spy;
            phi = 2.0 - spx / spy;
        }
    }
    else
    {
        if(spx < spy)
        {
            r = -spx;
            phi = 4.0f + spy / spx;
        }
        else
        {
            r = -spy;

            if(spy != 0.0)
                phi = 6.0 - spx / spy;
            else
                phi = 0.0;
        }
    }

    phi *= PI / 4.0;


    return vec3(r * cos(phi), r * sin(phi), 0.0f);
}

vec3 random_in_unit_sphere()
{
    float phi = 2.0 * PI * rand2D();
    float cosTheta = 2.0 * rand2D() - 1.0;
    float u = rand2D();

    float theta = acos(cosTheta);
    float r = pow(u, 1.0 / 3.0);

    float x = r * sin(theta) * cos(phi);
    float y = r * sin(theta) * sin(phi);
    float z = r * cos(theta);

    return vec3(x, y, z);
}

// Initialize camera
Camera Camera_initFromViewMatrix(mat4 viewMatrix, vec3 position, float fovDegrees, float aspect) {
    Camera camera;
    camera.origin = position;
    
    // Set camera vectors from view matrix
    camera.w = -normalize(vec3(viewMatrix[0][2], viewMatrix[1][2], viewMatrix[2][2]));
    camera.u = normalize(vec3(viewMatrix[0][0], viewMatrix[1][0], viewMatrix[2][0]));
    camera.v = normalize(vec3(viewMatrix[0][1], viewMatrix[1][1], viewMatrix[2][1]));
    
    // Set lens parameters
    float aperture = 0.1;
    camera.lensRadius = aperture / 2.0;
    
    // Calculate frustum
    float distToFocus = 10.0;
    float theta = fovDegrees * PI / 180.0;
    float halfHeight = tan(theta / 2.0);
    float halfWidth = aspect * halfHeight;
    
    camera.lowerLeftCorner = camera.origin - halfWidth * distToFocus * camera.u
                                          - halfHeight * distToFocus * camera.v
                                          - distToFocus * camera.w;
    camera.horizontal = 2.0 * halfWidth * distToFocus * camera.u;
    camera.vertical = 2.0 * halfHeight * distToFocus * camera.v;
    
    return camera;
}

// Generate a ray from camera
Ray Camera_getRay(Camera camera, float s, float t) {
    // Add a very small jitter to help with anti-aliasing edges
    float pixelRadius = 0.5 / max(iResolution.x, iResolution.y);
    float jitterX = pixelRadius * (rand2D() - 0.5);
    float jitterY = pixelRadius * (rand2D() - 0.5);
    
    vec3 rd = camera.lensRadius * random_in_unit_disk();
    vec3 offset = camera.u * rd.x + camera.v * rd.y;
    
    Ray ray;
    ray.origin = camera.origin + offset;
    ray.direction = normalize(camera.lowerLeftCorner + 
                    (s + jitterX) * camera.horizontal + 
                    (t + jitterY) * camera.vertical - 
                    camera.origin - offset);
    return ray;
}

// Ray-sphere intersection
bool Sphere_hit(int sphereIdx, Ray ray, float t_min, float t_max, inout IntersectInfo rec) {
    vec3 center = spheres[sphereIdx].xyz;
    float radius = spheres[sphereIdx].w;
    
    vec3 oc = ray.origin - center;
    float a = dot(ray.direction, ray.direction);
    float b = dot(oc, ray.direction);
    float c = dot(oc, oc) - radius * radius;
    float discriminant = b * b - a * c;
    
    if (discriminant > 0.0) {
        float temp = (-b - sqrt(discriminant)) / a;
        if (temp < t_max && temp > t_min) {
            rec.t = temp;
            rec.point = ray.origin + temp * ray.direction;
            rec.normal = (rec.point - center) / radius;
            
            // Material properties
            rec.materialType = int(sphereData[sphereIdx].x);
            rec.albedo = sphereData[sphereIdx].yzw;
            rec.fuzz = sphereData2[sphereIdx].x;
            rec.refractionIndex = sphereData2[sphereIdx].y;
            
            return true;
        }
        
        temp = (-b + sqrt(discriminant)) / a;
        if (temp < t_max && temp > t_min) {
            rec.t = temp;
            rec.point = ray.origin + temp * ray.direction;
            rec.normal = (rec.point - center) / radius;
            
            // Material properties
            rec.materialType = int(sphereData[sphereIdx].x);
            rec.albedo = sphereData[sphereIdx].yzw;
            rec.fuzz = sphereData2[sphereIdx].x;
            rec.refractionIndex = sphereData2[sphereIdx].y;
            
            return true;
        }
    }
    return false;
}

// Ray-box intersection for octree
bool rayBoxIntersection(Ray ray, vec3 boxMin, vec3 boxMax, out float tmin, out float tmax) {
    vec3 invDir = 1.0 / ray.direction;
    vec3 tbot = invDir * (boxMin - ray.origin);
    vec3 ttop = invDir * (boxMax - ray.origin);
    
    vec3 tmin3 = min(tbot, ttop);
    vec3 tmax3 = max(tbot, ttop);
    
    tmin = max(max(tmin3.x, tmin3.y), tmin3.z);
    tmax = min(min(tmax3.x, tmax3.y), tmax3.z);
    
    return tmax >= tmin;
}

bool traverseOctree(Ray ray, float t_min, float t_max, inout IntersectInfo rec) {
    const int MAX_STACK = 200;
    int nodeStack[MAX_STACK];
    float tminStack[MAX_STACK];
    float tmaxStack[MAX_STACK];
    
    
    // represents the root
    int stackPtr = 0;
    nodeStack[0] = 0;
    tminStack[0] = t_min;
    tmaxStack[0] = t_max;
    
    bool hit_anything = false;
    float closest_so_far = t_max;
    
    // If the stack is empty, it's finished
    while (stackPtr >= 0) {
        int nodeIdx = nodeStack[stackPtr];
        float node_tmin = tminStack[stackPtr];
        float node_tmax = tmaxStack[stackPtr];
        stackPtr--;
        
        // if we found already a closer node, continue
        if (node_tmin > closest_so_far) continue;
        
        // get data
        vec4 node1 = octreeNodes[nodeIdx];
        vec4 node2 = octreeNodes2[nodeIdx];
        vec3 nodeMin = node1.xyz;
        vec3 nodeMax = node2.xyz;
        int childrenOffset = int(node1.w);
        int objectsOffset = int(node2.w);
        int objectCount = octreeNodeCounts[nodeIdx];
        
        // Test for intersection
        float boxTMin, boxTMax;
        if (!rayBoxIntersection(ray, nodeMin, nodeMax, boxTMin, boxTMax) || boxTMax < node_tmin || boxTMin > node_tmax) {
            continue; // no intersection
        }
        
        // leaf node
        if (childrenOffset == -1) {

            for (int i = 0; i < objectCount; i++) {
                int sphereIdx = objectIndices[objectsOffset + i];
                
                IntersectInfo temp_rec;
                if (Sphere_hit(sphereIdx, ray, node_tmin, closest_so_far, temp_rec)) {
                    hit_anything = true;
                    closest_so_far = temp_rec.t;
                    rec = temp_rec;
                }
            }
        } 
        else {
            // Push children in front-to-back order
            // TODO: maybe sort based on ray direction
            for (int i = 0; i < 8; i++) {
                int childIdx = childrenOffset + i;
                if (childIdx >= octreeNodeCount) continue;
                
                if (stackPtr < MAX_STACK - 1) {
                    stackPtr++;
                    nodeStack[stackPtr] = childIdx;
                    tminStack[stackPtr] = node_tmin;
                    tmaxStack[stackPtr] = min(node_tmax, closest_so_far);
                }
            }
        }
    }
    
    return hit_anything;
}

bool bruteForceIntersect(Ray ray, float t_min, float t_max, inout IntersectInfo rec) {
    IntersectInfo temp_rec;
    bool hit_anything = false;
    float closest_so_far = t_max;
    
    for (int i = 0; i < sphereCount; i++) {
        if (Sphere_hit(i, ray, t_min, closest_so_far, temp_rec)) {
            hit_anything = true;
            closest_so_far = temp_rec.t;
            rec = temp_rec;
        }
    }
    
    return hit_anything;
}

bool intersectScene(Ray ray, float t_min, float t_max, inout IntersectInfo rec) {
    if (useOctree == 1) {
        return traverseOctree(ray, t_min, t_max, rec);
    } else {
        return bruteForceIntersect(ray, t_min, t_max, rec);
    }
}

bool refractVec(vec3 v, vec3 n, float ni_over_nt, out vec3 refracted) {
    vec3 uv = normalize(v);
    float dt = dot(uv, n);
    float discriminant = 1.0 - ni_over_nt * ni_over_nt * (1.0 - dt * dt);
    if (discriminant > 0.0) {
        refracted = ni_over_nt * (uv - n * dt) - n * sqrt(discriminant);
        return true;
    }
    return false;
}

float schlick(float cosine, float refractionIndex) {
    float r0 = (1.0 - refractionIndex) / (1.0 + refractionIndex);
    r0 = r0 * r0;
    return r0 + (1.0 - r0) * pow(1.0 - cosine, 5.0);
}

bool Material_bsdf(IntersectInfo isectInfo, Ray wo, inout Ray wi, inout vec3 attenuation) {
    int materialType = isectInfo.materialType;
    wi.origin = isectInfo.point;

    switch (materialType) {
        case LAMBERT:
            vec3 target = isectInfo.point + isectInfo.normal + random_in_unit_sphere();
            wi.direction = normalize(target - isectInfo.point);
            attenuation = isectInfo.albedo;
            return true;

        case METAL:
            float fuzz = isectInfo.fuzz;
            vec3 reflected = reflect(normalize(wo.direction), isectInfo.normal);
            wi.direction = reflected + fuzz * random_in_unit_sphere();
            attenuation = isectInfo.albedo;
            return (dot(wi.direction, isectInfo.normal) > 0.0f);

        case DIELECTRIC:
            vec3 outward_normal;
            float ni_over_nt;
            float cosine;
            float rafractionIndex = isectInfo.refractionIndex;
            attenuation = vec3(1.0f);
            
            // Determine if we're entering or exiting the material
            if (dot(wo.direction, isectInfo.normal) > 0.0f) {
                outward_normal = -isectInfo.normal;
                ni_over_nt = rafractionIndex;
                cosine = dot(wo.direction, isectInfo.normal) / length(wo.direction);
                cosine = sqrt(1.0f - rafractionIndex * rafractionIndex * (1.0f - cosine * cosine));
            } else {
                outward_normal = isectInfo.normal;
                ni_over_nt = 1.0f / rafractionIndex;
                cosine = -dot(wo.direction, isectInfo.normal) / length(wo.direction);
            }
            
            // Calculate reflection probability
            float reflect_prob;
            vec3 refracted;
            bool can_refract = refractVec(wo.direction, outward_normal, ni_over_nt, refracted);
            reflect_prob = can_refract ? schlick(cosine, rafractionIndex) : 1.0f;
            
            if (rand2D() < reflect_prob) {
                // Reflection path
                wi.direction = reflect(wo.direction, isectInfo.normal);
            } else {
                // We already calculated refraction above if possible
                wi.direction = refracted;
            }
            
            return true;
        default:
            return false;
    }

    
    return false;
}

// Sky color for rays that don't hit anything
vec3 skyColor(Ray ray) {
    vec3 unit_direction = normalize(ray.direction);
    float t = 0.5 * (unit_direction.y + 1.0);
    return (1.0 - t) * vec3(1.0, 1.0, 1.0) + t * vec3(0.5, 0.7, 1.0);
}

// Main radiance computation with octree traversal
vec3 radiance(Ray ray) {
    IntersectInfo rec;
    vec3 col = vec3(1.0, 1.0, 1.0);
    float importance = 1.0;
    
    for (int i = 0; i < maxDepth; i++) {
        if (importance < 0.01) break;

        if (intersectScene(ray, 0.001, MAXFLOAT, rec)) {
            Ray wi;
            vec3 attenuation;
            
            bool wasScattered = Material_bsdf(rec, ray, wi, attenuation);

            ray.origin = wi.origin;
            ray.direction = wi.direction;
            
            if (wasScattered)
                col *= attenuation;
            else {
                col *= vec3(0.0, 0.0, 0.0);
                break;
            }
            importance *= max(attenuation.r, max(attenuation.g, attenuation.b));
        }
        else {
            col *= skyColor(ray);
            break;
        }

        
    }
    
    return col;
}

void main() {
    // Initialize camera
    Camera camera = Camera_initFromViewMatrix(view, cameraPosition, cameraZoom, float(iResolution.x) / float(iResolution.y));
    
    // Initialize random state
    randState = FragCoord.xy / iResolution.xy;
    
    // Accumulate samples
    vec3 col = vec3(0.0, 0.0, 0.0);
    
    #pragma unroll // TODO: Idk if this helps
    for (int s = 0; s < numSamples; s++) {
        int sqrt_ns = int(sqrt(float(numSamples)));
        int i = s % sqrt_ns;
        int j = s / sqrt_ns;
        
        float u = float(FragCoord.x + (float(i) + rand2D()) / float(sqrt_ns)) / float(iResolution.x);
        float v = float(FragCoord.y + (float(j) + rand2D()) / float(sqrt_ns)) / float(iResolution.y);
        
        Ray r = Camera_getRay(camera, u, v);
        col += radiance(r);
    }
    
    // Average samples and gamma correct
    col /= float(numSamples);
    // Power 2.2 gamma correction
    col = pow(col, vec3(1.0/2.2));
    
    FragColor = vec4(col, 1.0);
}