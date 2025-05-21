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

// Resolution
uniform vec3 iResolution;

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
    int octreeObjectCounts[];
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
    uint state = uint(randState.x * 1664525u + randState.y * 1013904223u) + 1013904223u;
    state = state ^ (state >> 16u);
    state *= 0x85ebca6bu;
    state = state ^ (state >> 13u);
    state *= 0xc2b2ae35u;
    state = state ^ (state >> 16u);
    
    randState.x = fract(randState.y * 1664525.0);
    randState.y = fract(float(state) / 4294967296.0);
    
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

vec3 random_in_unit_sphere(){
    float z = 2.0 * rand2D() - 1.0;
    float phi = 2.0 * PI * rand2D();
    float r = pow(rand2D(), 1.0/3.0);
    float sqrt1minz2 = sqrt(1.0 - z*z);
    return vec3(
        r * sqrt1minz2 * cos(phi),
        r * sqrt1minz2 * sin(phi),
        r * z
    );
}

vec3 random_cosine_direction() {
    // More efficient for diffuse materials
    float r1 = rand2D();
    float r2 = rand2D();
    float phi = 2.0 * PI * r1;
    
    float sqrt_r2 = sqrt(r2);
    float x = cos(phi) * sqrt_r2;
    float y = sin(phi) * sqrt_r2;
    float z = sqrt(1.0 - r2);
    
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
    float half_b = dot(oc, ray.direction);
    float c = dot(oc, oc) - radius * radius;
    float discriminant = half_b * half_b - a * c;
    
    if (discriminant > 0.0) {
        float sqrtd = sqrt(discriminant);
        
        float temp = (-half_b - sqrtd) / a;
        if (temp < t_max && temp > t_min) {
            rec.t = temp;
            rec.point = ray.origin + temp * ray.direction;
            rec.normal = (rec.point - center) / radius;
            
            vec4 matData = sphereData[sphereIdx];
            rec.materialType = int(matData.x);
            rec.albedo = matData.yzw;
            
            vec4 matData2 = sphereData2[sphereIdx];
            rec.fuzz = matData2.x;
            rec.refractionIndex = matData2.y;
            
            return true;
        }
        
        temp = (-half_b + sqrtd) / a;
        if (temp < t_max && temp > t_min) {
            rec.t = temp;
            rec.point = ray.origin + temp * ray.direction;
            rec.normal = (rec.point - center) / radius;
            
            vec4 matData = sphereData[sphereIdx];
            rec.materialType = int(matData.x);
            rec.albedo = matData.yzw;

            vec4 matData2 = sphereData2[sphereIdx];
            rec.fuzz = matData2.x;
            rec.refractionIndex = matData2.y;
            
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
    
    int stackPtr = 0;
    nodeStack[0] = 0;
    tminStack[0] = t_min;
    tmaxStack[0] = t_max;
    
    bool hit_anything = false;
    float closest_so_far = t_max;

    while (stackPtr >= 0) {
        int nodeIdx = nodeStack[stackPtr];
        float node_tmin = tminStack[stackPtr];
        float node_tmax = tmaxStack[stackPtr--];
        
        // Early termination if we can't improve on existing hit
        if (node_tmin > closest_so_far) continue;
        
        // Get node
        vec4 node1 = octreeNodes[nodeIdx];
        vec4 node2 = octreeNodes2[nodeIdx];
        vec3 nodeMin = node1.xyz;
        vec3 nodeMax = node2.xyz;
        int childrenOffset = int(node1.w);
        int objectsOffset = int(node2.w);
        int objectCount = octreeObjectCounts[nodeIdx];
        
        // Test objects in leaf nodes
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
            // Process child nodes in order of distance from ray origin
            // For non-leaf nodes, add children to stack in best order
            
            // Your existing bit mask approach for ordering traversal is good,
            // but we can enhance it with distance-based prioritization
            int octantMask = 0;
            if (ray.direction.x < 0.0) octantMask |= 2;  // Flip bit 1 (X axis)
            if (ray.direction.y < 0.0) octantMask |= 4;  // Flip bit 2 (Y axis)
            if (ray.direction.z < 0.0) octantMask |= 1;  // Flip bit 0 (Z axis)
            
            // Process children in the optimal order (furthest to closest on stack)
            // This ensures we process closest nodes first when popping
            for (int i = 0; i < 8; i++) {
                int octant = (i ^ octantMask);
                int childIdx = childrenOffset + octant;
                
                if (childIdx >= octreeNodeCount) continue;
                
                // Get child bounds
                vec4 childNode1 = octreeNodes[childIdx];
                vec4 childNode2 = octreeNodes2[childIdx];
                vec3 childMin = childNode1.xyz;
                vec3 childMax = childNode2.xyz;
                
                // Check if child intersects ray before adding to stack
                float childTMin, childTMax;
                if (!rayBoxIntersection(ray, childMin, childMax, childTMin, childTMax) || 
                    childTMax < node_tmin || childTMin > closest_so_far) {
                    continue; // Skip non-intersecting children
                }
                
                if (stackPtr < MAX_STACK - 1) {
                    stackPtr++;
                    nodeStack[stackPtr] = childIdx;
                    tminStack[stackPtr] = max(childTMin, node_tmin);
                    tmaxStack[stackPtr] = min(childTMax, closest_so_far);
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
            vec3 local_dir = random_cosine_direction();

            // Transform from local to world space
            vec3 w = isectInfo.normal;
            vec3 u = normalize(cross((abs(w.x) > 0.1 ? vec3(0, 1, 0) : vec3(1, 0, 0)), w));
            vec3 v = cross(w, u);
            
            wi.direction = normalize(local_dir.x * u + local_dir.y * v + local_dir.z * w);
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
    float t = 0.5 * (ray.direction.y + 1.0);
    return (1.0 - t) * vec3(1.0, 1.0, 1.0) + t * vec3(0.5, 0.7, 1.0);
}

vec3 radiance(Ray ray) {
    IntersectInfo rec;
    vec3 col = vec3(1.0, 1.0, 1.0);
    float importance = 1.0;

    ray.direction = normalize(ray.direction);
    
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

/*
void main() {
    // Initialize camera
    Camera camera = Camera_initFromViewMatrix(view, cameraPosition, cameraZoom, float(iResolution.x) / float(iResolution.y));
    
    // Initialize random state with better seed
    randState = FragCoord.xy / iResolution.xy;
    
    // Stratified sampling - better distribution of samples
    vec3 col = vec3(0.0);
    int sqrt_ns = int(sqrt(float(numSamples)));
    
    // Use stratified sampling for better convergence
    for (int j = 0; j < sqrt_ns; j++) {
        for (int i = 0; i < sqrt_ns; i++) {
            // Stratified sample position
            float u = float(FragCoord.x + (float(i) + rand2D()) / float(sqrt_ns)) / float(iResolution.x);
            float v = float(FragCoord.y + (float(j) + rand2D()) / float(sqrt_ns)) / float(iResolution.y);
            
            Ray r = Camera_getRay(camera, u, v);
            col += radiance(r);
        }
    }
    
    // Average samples
    col /= float(numSamples);
    
    // Tone mapping - improves visual quality and handles HDR
    col = col / (col + vec3(1.0)); // Reinhard tone mapping
    
    // Gamma correction
    col = pow(col, vec3(1.0/2.2));
    
    FragColor = vec4(col, 1.0);
}*/