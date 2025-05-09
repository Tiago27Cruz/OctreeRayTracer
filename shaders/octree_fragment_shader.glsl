#version 330 core

#define NUMRAYS 10


out vec4 FragColor;

in vec2 FragCoord; 

uniform mat4      view;                  // View Matrix
uniform vec3 cameraPosition;    // Add a new uniform for camera position
uniform float cameraZoom;       // Add a new uniform for camera FOV/zoom

uniform vec3      iResolution;           // viewport resolution (in pixels)
uniform float     iTime;                 // shader playback time (in seconds)
uniform float     iTimeDelta;            // render time (in seconds)
uniform float     iFrameRate;            // shader frame rate
uniform int       iFrame;                // shader playback frame
uniform float     iChannelTime[4];       // channel playback time (in seconds)
uniform vec3      iChannelResolution[4]; // channel resolution (in pixels)
uniform vec4      iMouse;                // mouse pixel coords. xy: current (if MLB down), zw: click
uniform vec4      iDate;                 // (year, month, day, time in seconds)
uniform float     iSampleRate;           // sound sample rate (i.e., 44100)

vec2 randState;

struct OctreeNode {
    bool isLeaf;
    vec3 min;               // Bounding box min corner
    vec3 max;               // Bounding box max corner
    int childrenOffset;     // Offset to children in flat array (-1 if leaf)
    int objectsOffset;      // Offset to object indices array (-1 if not leaf)
    int objectCount;        // Number of objects in this node
};
struct Octree {
    OctreeNode root;
    int maxDepth;
};
struct Ray {
    vec3 origin;
    vec3 direction;
};
struct Sphere {
    vec3 center;
    float radius;
};
struct Camera {
    vec3 origin;
    vec3 lowerLeftCorner;
    vec3 horizontal;
    vec3 vertical;
    vec3 u, v, w;
    float lensRadius;
};

Camera initCamera(){
    Camera cam;

    cam.origin = cameraPosition;

    // Set camera vectors
    cam.w = normalize(-vec3(view[0][2], view[1][2], view[2][2]));
    cam.u = normalize(vec3(view[0][0], view[1][0], view[2][0]));
    cam.v = normalize(vec3(view[0][1], view[1][1], view[2][1]));

    // Set lens parameters
    float aperture = 0.1;
    cam.lensRadius = aperture / 2.0;

    // Set up the camera frustum
    float distToFocus = 10.0;
    
    float theta = fovDegrees * PI / 180.0;
    float halfHeight = tan(theta / 2.0);
    float aspect = float(iResolution.x) / float(iResolution.y);
    float halfWidth = aspect * halfHeight;
    
    cam.lowerLeftCorner = cam.origin - halfWidth * distToFocus * cam.u
                                          - halfHeight * distToFocus * cam.v
                                          - distToFocus * cam.w;

    cam.horizontal = 2.0 * halfWidth * distToFocus * cam.u;
    cam.vertical = 2.0 * halfHeight * distToFocus * cam.v;

    return cam;
}

Octree createScene(){
    Octree octree;

    OctreeNode rootNode;


    octree.root = rootNode;

}

Ray generateRay(float x, float y, Camera cam){
    Ray ray;
    ray.origin = cam.origin;
    ray.direction = cam.lowerLeftCorner + x * cam.horizontal + y * cam.vertical - cam.origin;
    return ray;
}

vec2 rand2D(){
    randState.x = fract(sin(dot(randState.xy, vec2(12.9898, 78.233))) * 43758.5453);
    randState.y = fract(sin(dot(randState.xy, vec2(12.9898, 78.233))) * 43758.5453);;
    
    return randState.x;
}

vec3 random_in_unit_disk(){
    float spx = 2.0 * rand2D() - 1.0;
    float spy = 2.0 * rand2D() - 1.0;

    float r, phi;

    if(spx > -spy){
        if(spx > spy){
            r = spx;
            phi = spy / spx;
        }
        else{
            r = spy;
            phi = 2.0 - spx / spy;
        }
    }
    else{
        if(spx < spy){
            r = -spx;
            phi = 4.0f + spy / spx;
        }else{
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

void main{
    Camera cam = initCamera();

    randState = FragCoord.xy / iResolution.xy;

    vec3 col = vec3(0.0, 0.0, 0.0);

    for (int s = 0; s < NUMRAYS; s++)
    {
        float u = float(FragCoord.x + rand2D()) / float(iResolution.x);
        float v = float(FragCoord.y + rand2D()) / float(iResolution.y);

        Ray r = Camera_getRay(camera, u, v);
        col += radiance(r);
    }

    col /= float(NUMRAYS);
    col = vec3( sqrt(col[0]), sqrt(col[1]), sqrt(col[2]) );

    FragColor = vec4(col, 1.0);
}