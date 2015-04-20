#ifndef LIGHTSOUCE_H
#define LIGHTSOUCE_H


#include "Vec3.h"
#include <vector>
#include <cmath>
#include "Mesh.h"
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
    Vec3f vectorDir; //add vectorDirection


    //methods
    Ray(Vec3f position = Vec3f(0.0, 0.0, 0.0), Direction direction = Direction(0.0, 0.0));

    Ray(Vec3f position = Vec3f(0.0, 0.0, 0.0), Vec3f vectorDir = Vec3f(0.0, 1.0, 0.0));
    bool intersect(Vec3f * triangle, Vec3f &result);

    bool raySceneIntersection(Mesh meshVT[], int nbObjs, float &minDist);

    //intersect_remake makes use of intersect2
    bool intersect_remake(Vec3f * triangle, Vec3f &result);

    //intersect2 is an atomic method
    //o is the position of the ray, w is its direction
    bool intersect2(Vec3f * triangleABC, Vec3f &o, Vec3f &w, Vec3f &result);

    //solve Mx + Ny = P, where M,N,P are Vec2f each
    bool solveLinear2(float M[], float N[], float P[], float result[]);

    //consider the sphere covering triangles meshes with center Vec3f c and radius float r
    //the ray with origin at Vec3f o, direction Vec3f w,
    //ray if intersect with sphere, return true and stock the intersection in Vec3f result
    bool intersect_Sphere(Vec3f &o, Vec3f &w, Vec3f &c, float &r, Vec3f &result);

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
