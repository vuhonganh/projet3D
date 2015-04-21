#ifndef RAYSOURCE_H
#define RAYSOURCE_H


#include "Vec3.h"
#include <vector>
#include <cmath>
#include "Ray.h"
#include "tiny_obj_loader.h"
using namespace std;

#define LIGHT_SOURCE_NORMAL 100

struct RaySource
{    
    //elements
    Vec3f position;
    Vec3f upVector;
    Vec3f lookAtPosition;
    Vec3f lightSource;
    float verticalAngle;
    float horizontalAngle;
    float distanceToScreen;
    int resolutionWidth;
    int resolutionHeight;
    vector <Ray> rays;
    
    //methods
    RaySource(Vec3f lightSource, Vec3f position, Vec3f lookAtPosition, int resolutionWidth, int resolutionHeight);
    
    void exportToRGB(const vector<tinyobj::shape_t> &shapes, 
                     const vector <tinyobj::material_t> &materials,
                     unsigned char * rayImage);
};

#endif // RAYSOURCE_H
