#version 330 core

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

struct OctreeNode {
    bool isLeaf;
    int children[8];
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

void main{
    Camera cam = initCamera();
    
}