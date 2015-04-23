#ifndef RAYSOURCE_H
#define RAYSOURCE_H


#include "Vec3.h"
#include <vector>
#include <cmath>
#include "Ray.h"
#include "tiny_obj_loader.h"
#include "BSHNode.h"
using namespace std;

#define LIGHT_SOURCE_NORMAL 100

struct RaySource
{    
    //elements
    Vec3f position;
    Vec3f upVector;
    Vec3f lookAtPosition;
    float verticalAngle;
    float horizontalAngle;
    float distanceToScreen;
    int resolutionWidth;
    int resolutionHeight;
    vector <Vec3f> lightSources;
    float * image;
    
    //methods
    RaySource(const vector <Vec3f> &lightSources, Vec3f position, Vec3f lookAtPosition, int resolutionWidth, int resolutionHeight);
    ~RaySource();
    void exportToRGB(const vector<tinyobj::shape_t> &shapes, 
                     const vector <tinyobj::material_t> &materials,
                     BSHNode * bshRoot,
                     unsigned char * rayImage);
};

#endif // RAYSOURCE_H
