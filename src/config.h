#ifndef CONFIG_H
#define CONFIG_H
#include <string>

// Debug mode
const int DEBUG = 0;
const int USEOCTREE = 1;

const int USEPREBUILT = 0;
const int NUMSPHERES = 100;

// Maximum depth for the octree
const int MAXDEPTH = 3;
const int DEBUGDEPTH = 3;

// Maximum number of spheres per node in the octree
const int MAXSPHERESPERNODE = 0;
const int DEBUGSPHERESPERNODE = 2;

// Number of rays incoming from the camera; The more rays, the more accurate the result
const int NUMSAMPLES = 16;

// How many times a ray can bounce before it is discarded
const int MAXRAYSDEPTH = 8;

// Screen resolution
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

const bool COLLECTSTATS = 0;

const std::string OUTPUTFILE = "stats.csv";

#endif // CONFIG_H