
#version 330 core
#define PI 			3.1415926535
#define MAXFLOAT	99999.99

#define MAXDEPTH 	5
#define NUMSAMPLES 	4

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
                



struct Ray
{
    vec3 origin;
    vec3 direction;
};


struct IntersectInfo
{
    // surface properties
    float t;
    vec3  p;
    vec3  normal;
	
    // material properties
    int   materialType;
    vec3  albedo;
    float fuzz;
    float refractionIndex;
};
    
    
struct Sphere
{
    // sphere properties
    vec3 center;
    float radius;

    // material
    int   materialType;
    vec3  albedo;
    float fuzz;
    float refractionIndex;
};

    
bool Sphere_hit(Sphere sphere, Ray ray, float t_min, float t_max, out IntersectInfo rec)
{
    vec3 oc = ray.origin - sphere.center;
    float a = dot(ray.direction, ray.direction);
    float b = dot(oc, ray.direction);
    float c = dot(oc, oc) - sphere.radius * sphere.radius;

    float discriminant = b * b - a * c;

    if (discriminant > 0.0f)
    {
        float temp = (-b - sqrt(discriminant)) / a;

        if (temp < t_max && temp > t_min)
        {
            rec.t                = temp;
            rec.p                = ray.origin + rec.t * ray.direction;
            rec.normal           = (rec.p - sphere.center) / sphere.radius;
            rec.materialType     = sphere.materialType;
            rec.albedo           = sphere.albedo;
            rec.fuzz             = sphere.fuzz;
            rec.refractionIndex  = sphere.refractionIndex;

            return true;
        }


        temp = (-b + sqrt(discriminant)) / a;

        if (temp < t_max && temp > t_min)
        {
            rec.t                = temp;
            rec.p                = ray.origin + rec.t * ray.direction;
            rec.normal           = (rec.p - sphere.center) / sphere.radius;
            rec.materialType     = sphere.materialType;
            rec.albedo           = sphere.albedo;
            rec.fuzz             = sphere.fuzz;
            rec.refractionIndex  = sphere.refractionIndex;

            return true;
        }
    }

    return false;
}


// The 'scene'
Sphere sceneList[] = Sphere[84](
Sphere(vec3( 0.000000, -1000.000000, 0.000000), 1000.000000, 0, vec3( 0.500000, 0.500000, 0.500000), 1.000000, 1.000000),
Sphere(vec3( -7.995381, 0.200000, -7.478668), 0.200000, 0, vec3( 0.380012, 0.506085, 0.762437), 1.000000, 1.000000),
Sphere(vec3( -7.696819, 0.200000, -5.468978), 0.200000, 0, vec3( 0.596282, 0.140784, 0.017972), 1.000000, 1.000000),
Sphere(vec3( -7.824804, 0.200000, -3.120637), 0.200000, 0, vec3( 0.288507, 0.465652, 0.665070), 1.000000, 1.000000),
Sphere(vec3( -7.132909, 0.200000, -1.701323), 0.200000, 0, vec3( 0.101047, 0.293493, 0.813446), 1.000000, 1.000000),
Sphere(vec3( -7.569523, 0.200000, 0.494554), 0.200000, 0, vec3( 0.365924, 0.221622, 0.058332), 1.000000, 1.000000),
Sphere(vec3( -7.730332, 0.200000, 2.358976), 0.200000, 0, vec3( 0.051231, 0.430547, 0.454086), 1.000000, 1.000000),
Sphere(vec3( -7.892865, 0.200000, 4.753728), 0.200000, 1, vec3( 0.826684, 0.820511, 0.908836), 0.389611, 1.000000),
Sphere(vec3( -7.656691, 0.200000, 6.888913), 0.200000, 0, vec3( 0.346542, 0.225385, 0.180132), 1.000000, 1.000000),
Sphere(vec3( -7.217835, 0.200000, 8.203466), 0.200000, 1, vec3( 0.600463, 0.582386, 0.608277), 0.427369, 1.000000),
Sphere(vec3( -5.115232, 0.200000, -7.980404), 0.200000, 0, vec3( 0.256969, 0.138639, 0.080293), 1.000000, 1.000000),
Sphere(vec3( -5.323222, 0.200000, -5.113037), 0.200000, 0, vec3( 0.193093, 0.510542, 0.613362), 1.000000, 1.000000),
Sphere(vec3( -5.410681, 0.200000, -3.527741), 0.200000, 0, vec3( 0.352200, 0.191551, 0.115972), 1.000000, 1.000000),
Sphere(vec3( -5.460670, 0.200000, -1.166543), 0.200000, 0, vec3( 0.029486, 0.249874, 0.077989), 1.000000, 1.000000),
Sphere(vec3( -5.457659, 0.200000, 0.363870), 0.200000, 0, vec3( 0.395713, 0.762043, 0.108515), 1.000000, 1.000000),
Sphere(vec3( -5.798715, 0.200000, 2.161684), 0.200000, 2, vec3( 0.000000, 0.000000, 0.000000), 1.000000, 1.500000),
Sphere(vec3( -5.116586, 0.200000, 4.470188), 0.200000, 0, vec3( 0.059444, 0.404603, 0.171767), 1.000000, 1.000000),
Sphere(vec3( -5.273591, 0.200000, 6.795187), 0.200000, 0, vec3( 0.499454, 0.131330, 0.158348), 1.000000, 1.000000),
Sphere(vec3( -5.120286, 0.200000, 8.731398), 0.200000, 0, vec3( 0.267365, 0.136024, 0.300483), 1.000000, 1.000000),
Sphere(vec3( -3.601565, 0.200000, -7.895600), 0.200000, 0, vec3( 0.027752, 0.155209, 0.330428), 1.000000, 1.000000),
Sphere(vec3( -3.735860, 0.200000, -5.163056), 0.200000, 1, vec3( 0.576768, 0.884712, 0.993335), 0.359385, 1.000000),
Sphere(vec3( -3.481116, 0.200000, -3.794556), 0.200000, 0, vec3( 0.405104, 0.066436, 0.009339), 1.000000, 1.000000),
Sphere(vec3( -3.866858, 0.200000, -1.465965), 0.200000, 0, vec3( 0.027570, 0.021652, 0.252798), 1.000000, 1.000000),
Sphere(vec3( -3.168870, 0.200000, 0.553099), 0.200000, 0, vec3( 0.421992, 0.107577, 0.177504), 1.000000, 1.000000),
Sphere(vec3( -3.428552, 0.200000, 2.627547), 0.200000, 1, vec3( 0.974029, 0.653443, 0.571877), 0.312780, 1.000000),
Sphere(vec3( -3.771736, 0.200000, 4.324785), 0.200000, 0, vec3( 0.685957, 0.000043, 0.181270), 1.000000, 1.000000),
Sphere(vec3( -3.768522, 0.200000, 6.384588), 0.200000, 0, vec3( 0.025972, 0.082246, 0.138765), 1.000000, 1.000000),
Sphere(vec3( -3.286992, 0.200000, 8.441148), 0.200000, 0, vec3( 0.186577, 0.560376, 0.367045), 1.000000, 1.000000),
Sphere(vec3( -1.552127, 0.200000, -7.728200), 0.200000, 0, vec3( 0.202998, 0.002459, 0.015350), 1.000000, 1.000000),
Sphere(vec3( -1.360796, 0.200000, -5.346098), 0.200000, 0, vec3( 0.690820, 0.028470, 0.179907), 1.000000, 1.000000),
Sphere(vec3( -1.287209, 0.200000, -3.735321), 0.200000, 0, vec3( 0.345974, 0.672353, 0.450180), 1.000000, 1.000000),
Sphere(vec3( -1.344859, 0.200000, -1.726654), 0.200000, 0, vec3( 0.209209, 0.431116, 0.164732), 1.000000, 1.000000),
Sphere(vec3( -1.974774, 0.200000, 0.183260), 0.200000, 0, vec3( 0.006736, 0.675637, 0.622067), 1.000000, 1.000000),
Sphere(vec3( -1.542872, 0.200000, 2.067868), 0.200000, 0, vec3( 0.192247, 0.016661, 0.010109), 1.000000, 1.000000),
Sphere(vec3( -1.743856, 0.200000, 4.752810), 0.200000, 0, vec3( 0.295270, 0.108339, 0.276513), 1.000000, 1.000000),
Sphere(vec3( -1.955621, 0.200000, 6.493702), 0.200000, 0, vec3( 0.270527, 0.270494, 0.202029), 1.000000, 1.000000),
Sphere(vec3( -1.350449, 0.200000, 8.068503), 0.200000, 1, vec3( 0.646942, 0.501660, 0.573693), 0.346551, 1.000000),
Sphere(vec3( 0.706123, 0.200000, -7.116040), 0.200000, 0, vec3( 0.027695, 0.029917, 0.235781), 1.000000, 1.000000),
Sphere(vec3( 0.897766, 0.200000, -5.938681), 0.200000, 0, vec3( 0.114934, 0.046258, 0.039647), 1.000000, 1.000000),
Sphere(vec3( 0.744113, 0.200000, -3.402960), 0.200000, 0, vec3( 0.513631, 0.335578, 0.204787), 1.000000, 1.000000),
Sphere(vec3( 0.867750, 0.200000, -1.311908), 0.200000, 0, vec3( 0.400246, 0.000956, 0.040513), 1.000000, 1.000000),
Sphere(vec3( 0.082480, 0.200000, 0.838206), 0.200000, 0, vec3( 0.594141, 0.215068, 0.025718), 1.000000, 1.000000),
Sphere(vec3( 0.649692, 0.200000, 2.525103), 0.200000, 1, vec3( 0.602157, 0.797249, 0.614694), 0.341860, 1.000000),
Sphere(vec3( 0.378574, 0.200000, 4.055579), 0.200000, 0, vec3( 0.005086, 0.003349, 0.064403), 1.000000, 1.000000),
Sphere(vec3( 0.425844, 0.200000, 6.098526), 0.200000, 0, vec3( 0.266812, 0.016602, 0.000853), 1.000000, 1.000000),
Sphere(vec3( 0.261365, 0.200000, 8.661150), 0.200000, 0, vec3( 0.150201, 0.007353, 0.152506), 1.000000, 1.000000),
Sphere(vec3( 2.814218, 0.200000, -7.751227), 0.200000, 1, vec3( 0.570094, 0.610319, 0.584192), 0.018611, 1.000000),
Sphere(vec3( 2.050073, 0.200000, -5.731364), 0.200000, 0, vec3( 0.109886, 0.029498, 0.303265), 1.000000, 1.000000),
Sphere(vec3( 2.020130, 0.200000, -3.472627), 0.200000, 0, vec3( 0.216908, 0.216448, 0.221775), 1.000000, 1.000000),
Sphere(vec3( 2.884277, 0.200000, -1.232662), 0.200000, 0, vec3( 0.483428, 0.027275, 0.113898), 1.000000, 1.000000),
Sphere(vec3( 2.644454, 0.200000, 0.596324), 0.200000, 0, vec3( 0.005872, 0.860718, 0.561933), 1.000000, 1.000000),
Sphere(vec3( 2.194283, 0.200000, 2.880603), 0.200000, 0, vec3( 0.452710, 0.824152, 0.045179), 1.000000, 1.000000),
Sphere(vec3( 2.281000, 0.200000, 4.094307), 0.200000, 0, vec3( 0.002091, 0.145849, 0.032535), 1.000000, 1.000000),
Sphere(vec3( 2.080841, 0.200000, 6.716384), 0.200000, 0, vec3( 0.468539, 0.032772, 0.018071), 1.000000, 1.000000),
Sphere(vec3( 2.287131, 0.200000, 8.583242), 0.200000, 2, vec3( 0.000000, 0.000000, 0.000000), 1.000000, 1.500000),
Sphere(vec3( 4.329136, 0.200000, -7.497218), 0.200000, 0, vec3( 0.030865, 0.071452, 0.016051), 1.000000, 1.000000),
Sphere(vec3( 4.502115, 0.200000, -5.941060), 0.200000, 2, vec3( 0.000000, 0.000000, 0.000000), 1.000000, 1.500000),
Sphere(vec3( 4.750631, 0.200000, -3.836759), 0.200000, 0, vec3( 0.702578, 0.084798, 0.141374), 1.000000, 1.000000),
Sphere(vec3( 4.082084, 0.200000, -1.180746), 0.200000, 0, vec3( 0.043052, 0.793077, 0.018707), 1.000000, 1.000000),
Sphere(vec3( 4.429173, 0.200000, 2.069721), 0.200000, 0, vec3( 0.179009, 0.147750, 0.617371), 1.000000, 1.000000),
Sphere(vec3( 4.277152, 0.200000, 4.297482), 0.200000, 0, vec3( 0.422693, 0.011222, 0.211945), 1.000000, 1.000000),
Sphere(vec3( 4.012743, 0.200000, 6.225072), 0.200000, 0, vec3( 0.986275, 0.073358, 0.133628), 1.000000, 1.000000),
Sphere(vec3( 4.047066, 0.200000, 8.419360), 0.200000, 1, vec3( 0.878749, 0.677170, 0.684995), 0.243932, 1.000000),
Sphere(vec3( 6.441846, 0.200000, -7.700798), 0.200000, 0, vec3( 0.309255, 0.342524, 0.489512), 1.000000, 1.000000),
Sphere(vec3( 6.047810, 0.200000, -5.519369), 0.200000, 0, vec3( 0.532361, 0.008200, 0.077522), 1.000000, 1.000000),
Sphere(vec3( 6.779211, 0.200000, -3.740542), 0.200000, 0, vec3( 0.161234, 0.539314, 0.016667), 1.000000, 1.000000),
Sphere(vec3( 6.430776, 0.200000, -1.332107), 0.200000, 0, vec3( 0.641951, 0.661402, 0.326114), 1.000000, 1.000000),
Sphere(vec3( 6.476387, 0.200000, 0.329973), 0.200000, 0, vec3( 0.033000, 0.648388, 0.166911), 1.000000, 1.000000),
Sphere(vec3( 6.568686, 0.200000, 2.116949), 0.200000, 0, vec3( 0.590952, 0.072292, 0.125672), 1.000000, 1.000000),
Sphere(vec3( 6.371189, 0.200000, 4.609841), 0.200000, 1, vec3( 0.870345, 0.753830, 0.933118), 0.233489, 1.000000),
Sphere(vec3( 6.011877, 0.200000, 6.569579), 0.200000, 0, vec3( 0.044868, 0.651697, 0.086779), 1.000000, 1.000000),
Sphere(vec3( 6.096087, 0.200000, 8.892333), 0.200000, 0, vec3( 0.588587, 0.078723, 0.044928), 1.000000, 1.000000),
Sphere(vec3( 8.185763, 0.200000, -7.191109), 0.200000, 1, vec3( 0.989702, 0.886784, 0.540759), 0.104229, 1.000000),
Sphere(vec3( 8.411960, 0.200000, -5.285309), 0.200000, 0, vec3( 0.139604, 0.022029, 0.461688), 1.000000, 1.000000),
Sphere(vec3( 8.047109, 0.200000, -3.427552), 0.200000, 1, vec3( 0.815002, 0.631228, 0.806757), 0.150782, 1.000000),
Sphere(vec3( 8.119639, 0.200000, -1.652587), 0.200000, 0, vec3( 0.177852, 0.429797, 0.042251), 1.000000, 1.000000),
Sphere(vec3( 8.818120, 0.200000, 0.401292), 0.200000, 0, vec3( 0.065416, 0.087694, 0.040518), 1.000000, 1.000000),
Sphere(vec3( 8.754155, 0.200000, 2.152549), 0.200000, 0, vec3( 0.230659, 0.035665, 0.435895), 1.000000, 1.000000),
Sphere(vec3( 8.595298, 0.200000, 4.802001), 0.200000, 0, vec3( 0.188493, 0.184933, 0.040215), 1.000000, 1.000000),
Sphere(vec3( 8.036216, 0.200000, 6.739752), 0.200000, 0, vec3( 0.023192, 0.364636, 0.464844), 1.000000, 1.000000),
Sphere(vec3( 8.256561, 0.200000, 8.129115), 0.200000, 0, vec3( 0.002612, 0.598319, 0.435378), 1.000000, 1.000000),
Sphere(vec3( 0.000000, 1.000000, 0.000000), 1.000000, 2, vec3( 0.000000, 0.000000, 0.000000), 1.000000, 1.500000),
Sphere(vec3( -4.000000, 1.000000, 0.000000), 1.000000, 0, vec3( 0.400000, 0.200000, 0.100000), 1.000000, 1.000000),
Sphere(vec3( 4.000000, 1.000000, 0.000000), 1.000000, 1, vec3( 0.700000, 0.600000, 0.500000), 0.000000, 1.000000)
);



// Schlick's approximation for approximating the contribution of the Fresnel factor
// in the specular reflection of light from a non-conducting surface between two media
//
// Theta is the angle between the direction from which the incident light is coming and
// the normal of the interface between the two media
float schlick(float cos_theta, float n2)
{
    const float n1 = 1.0f;  // refraction index for air

    float r0s = (n1 - n2) / (n1 + n2);
    float r0 = r0s * r0s;

    return r0 + (1.0f - r0) * pow((1.0f - cos_theta), 5.0f);
}


bool refractVec(vec3 v, vec3 n, float ni_over_nt, out vec3 refracted)
{
    vec3 uv = normalize(v);

    float dt = dot(uv, n);

    float discriminant = 1.0 - ni_over_nt * ni_over_nt * (1.0f - dt * dt);

    if (discriminant > 0.0f)
    {
        refracted = ni_over_nt*(uv - n * dt) - n * sqrt(discriminant);

        return true;
    }
    else
        return false;
}


vec3 reflectVec(vec3 v, vec3 n)
{
     return v - 2.0f * dot(v, n) * n;
}



// random number generator
vec2 randState;

float hash( const float n ) 
{
     return fract(sin(n)*43758.54554213);
}


float rand2D()
{
    randState.x = fract(sin(dot(randState.xy, vec2(12.9898, 78.233))) * 43758.5453);
    randState.y = fract(sin(dot(randState.xy, vec2(12.9898, 78.233))) * 43758.5453);;
    
    return randState.x;
}


// random direction in unit sphere (for lambert brdf)
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


// random point on unit disk (for depth of field camera)
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


struct Camera
{
    vec3 origin;
    vec3 lowerLeftCorner;
    vec3 horizontal;
    vec3 vertical;
    vec3 u, v, w;
    float lensRadius;
};

void Camera_initFromViewMatrix(out Camera camera, mat4 viewMatrix, vec3 position, float fovDegrees, float aspect)
{
    // Extract camera vectors
    camera.origin = position;
    
    // Extract camera orientation from view matrix (normalized)
    camera.w = normalize(-vec3(viewMatrix[0][2], viewMatrix[1][2], viewMatrix[2][2]));
    camera.u = normalize(vec3(viewMatrix[0][0], viewMatrix[1][0], viewMatrix[2][0]));
    camera.v = normalize(vec3(viewMatrix[0][1], viewMatrix[1][1], viewMatrix[2][1]));
    
    // Set lens parameters
    float aperture = 0.1;
    camera.lensRadius = aperture / 2.0;
    
    // Set up the camera frustum
    float distToFocus = 10.0;
    
    float theta = fovDegrees * PI / 180.0;
    float halfHeight = tan(theta / 2.0);
    float halfWidth = aspect * halfHeight;
    
    camera.lowerLeftCorner = camera.origin - halfWidth * distToFocus * camera.u
                                          - halfHeight * distToFocus * camera.v
                                          - distToFocus * camera.w;
                                          
    camera.horizontal = 2.0 * halfWidth * distToFocus * camera.u;
    camera.vertical = 2.0 * halfHeight * distToFocus * camera.v;
}

// uniform camera_up

Ray Camera_getRay(Camera camera, float s, float t)
{
    vec3 rd = camera.lensRadius * random_in_unit_disk();
    vec3 offset = camera.u * rd.x + camera.v * rd.y;

    Ray ray;

    ray.origin = camera.origin + offset;
    ray.direction = camera.lowerLeftCorner + s * camera.horizontal + t * camera.vertical - camera.origin - offset;

    return ray;
}


#define LAMBERT    0
#define METAL      1
#define DIELECTRIC 2

    

bool Material_bsdf(IntersectInfo isectInfo, Ray wo, out Ray wi, out vec3 attenuation)
{
    int materialType = isectInfo.materialType;

    if(materialType == LAMBERT)
    {
        vec3 target = isectInfo.p + isectInfo.normal + random_in_unit_sphere();

        wi.origin = isectInfo.p;
        wi.direction = target - isectInfo.p;

        attenuation = isectInfo.albedo;

        return true;
    }
    else
    if(materialType == METAL)
    {
        float fuzz = isectInfo.fuzz;

        vec3 reflected = reflect(normalize(wo.direction), isectInfo.normal);

        wi.origin = isectInfo.p;
        wi.direction = reflected + fuzz * random_in_unit_sphere();

        attenuation = isectInfo.albedo;

        return (dot(wi.direction, isectInfo.normal) > 0.0f);
    }
    else
    if(materialType == DIELECTRIC)
    {
        vec3 outward_normal;
        vec3 reflected = reflect(wo.direction, isectInfo.normal);

        float ni_over_nt;

        attenuation = vec3(1.0f, 1.0f, 1.0f);
        vec3 refracted;
        float reflect_prob;
        float cosine;

        float rafractionIndex = isectInfo.refractionIndex;

        if (dot(wo.direction, isectInfo.normal) > 0.0f)
        {
            outward_normal = -isectInfo.normal;
            ni_over_nt = rafractionIndex;
           
            cosine = dot(wo.direction, isectInfo.normal) / length(wo.direction);
            cosine = sqrt(1.0f - rafractionIndex * rafractionIndex * (1.0f - cosine * cosine));
        }
        else
        {
            outward_normal = isectInfo.normal;
            ni_over_nt = 1.0f / rafractionIndex;
            cosine = -dot(wo.direction, isectInfo.normal) / length(wo.direction);
        }
        if (refractVec(wo.direction, outward_normal, ni_over_nt, refracted))
            reflect_prob = schlick(cosine, rafractionIndex);
        else
            reflect_prob = 1.0f;
        if (rand2D() < reflect_prob)
        {
            wi.origin = isectInfo.p;
            wi.direction = reflected;
        }
        else
        {
            wi.origin = isectInfo.p;
            wi.direction = refracted;
        }

        return true;
    }

    return false;
}


bool intersectScene(Ray ray, float t_min, float t_max, out IntersectInfo rec)
{
        IntersectInfo temp_rec;

        bool hit_anything = false;
        float closest_so_far = t_max;

        for (int i = 0; i < sceneList.length(); i++)
        {
            Sphere sphere = sceneList[i];

            if (Sphere_hit(sphere, ray, t_min, closest_so_far, temp_rec))
            {
                hit_anything   = true;
                closest_so_far = temp_rec.t;
                rec            = temp_rec;
            }
        }

        return hit_anything;
}


vec3 skyColor(Ray ray)
{
    vec3 unit_direction = normalize(ray.direction);
    float t = 0.5 * (unit_direction.y + 1.0);

    return (1.0 - t) * vec3(1.0, 1.0, 1.0) + t * vec3(0.5, 0.7, 1.0);
}


vec3 radiance(Ray ray)
{
    IntersectInfo rec;

    vec3 col = vec3(1.0, 1.0, 1.0);

    for(int i = 0; i < MAXDEPTH; i++)
    {
        if (intersectScene(ray, 0.001, MAXFLOAT, rec))
        {
            Ray wi;
            vec3 attenuation;

            bool wasScattered = Material_bsdf(rec, ray, wi, attenuation);

            ray.origin = wi.origin;
            ray.direction = wi.direction;

            if (wasScattered)
                col *= attenuation;
            else
            {
                col *= vec3(0.0f, 0.0f, 0.0f);
                break;
            }
        }
        else
        {
            col *= skyColor(ray);
            break;
        }
    }

    return col;
}



void main()
{   
    // Instead of hardcoding camera parameters, use the view matrix
    Camera camera;
    
    // Use the uniforms passed from C++
    Camera_initFromViewMatrix(camera, view, cameraPosition, cameraZoom, 
                            float(iResolution.x) / float(iResolution.y));

    randState = FragCoord.xy / iResolution.xy;
    
    vec3 col = vec3(0.0, 0.0, 0.0);

    for (int s = 0; s < NUMSAMPLES; s++)
    {
        float u = float(FragCoord.x + rand2D()) / float(iResolution.x);
        float v = float(FragCoord.y + rand2D()) / float(iResolution.y);

        Ray r = Camera_getRay(camera, u, v);
        col += radiance(r);
    }

    col /= float(NUMSAMPLES);
    col = vec3( sqrt(col[0]), sqrt(col[1]), sqrt(col[2]) );

    FragColor = vec4(col, 1.0);
}