#ifndef LIGHTSOUCE_H
#define LIGHTSOUCE_H


#include "Vec3.h"
#include <vector>
#include <cmath>
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
    bool intersect_remake(Vec3f * triangle, Vec3f &result);

    bool intersect2(Vec3f * triangleABC, Vec3f &o, Vec3f &w, Vec3f &result);

    //solve Mx + Ny = P, where M,N,P are Vec2f each
    bool solveLinear2(float M[], float N[], float P[], float result[]);

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
