#ifndef LIGHTSOUCE_H
#define LIGHTSOUCE_H

#include "Vec3.h"
#include <vector>
using namespace std;

#define LIGHT_SOURCE_NORMAL 100

struct Direction
{
    //elements
    float angleXZ;
    float angleY;
    
    //methods
    Direction(float angleXZ = 0, float angleY = 0);
    Vec3f toVector();
};

struct Ray
{
    //elements
    Vec3f position;
    Direction direction;
        
    //methods
    Ray(Vec3f position = Vec3f(0.0, 0.0, 0.0), Direction direction = Direction(0.0, 0.0));
    bool intersect(Vec3f * triangle, Vec3f &result);
};

struct LightSource
{    
    //elements
    Vec3f position;
    vector <Ray> rays;
    
    //methods
    LightSource(int type = LIGHT_SOURCE_NORMAL, Vec3f position = Vec3f(0.0, 0.0, 0.0));
};

#endif // LIGHTSOUCE_H
