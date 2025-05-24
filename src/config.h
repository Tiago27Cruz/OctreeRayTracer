#ifndef CONFIG_H
#define CONFIG_H

// Debug mode
const int DEBUG = 0;
const int USEOCTREE = 1;

const int USEPREBUILT = 0;
const int NUMSPHERES = 2000;

// Maximum depth for the octree
const int MAXDEPTH = 4000;
const int DEBUGDEPTH = 3;

// Maximum number of spheres per node in the octree
const int MAXSPHERESPERNODE = 1;
const int DEBUGSPHERESPERNODE = 2;

// Number of rays incoming from the camera; The more rays, the more accurate the result
const int NUMSAMPLES = 4;

// How many times a ray can bounce before it is discarded
const int MAXRAYSDEPTH = 4;

// Screen resolution
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

const bool COLLECTSTATS = 1;

#endif // CONFIG_H