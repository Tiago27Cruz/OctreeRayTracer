#ifndef CONFIG_H
#define CONFIG_H

// Debug mode
const int DEBUG = 0;

// Maximum depth for the octree
const int MAXDEPTH = 8;
const int DEBUGDEPTH = 3;

// Maximum number of spheres per node in the octree
const int MAXSPHERESPERNODE = 4;
const int DEBUGSPHERESPERNODE = 2;

// Number of rays incoming from the camera; The more rays, the more accurate the result
const int NUMSAMPLES = 8;

// How many times a ray can bounce before it is discarded
const int MAXRAYSDEPTH = 4;

// Screen resolution
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

#endif // CONFIG_H