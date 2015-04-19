#ifndef RAY_H
#define RAY_H

#include "Vec3.h"
#include "tiny_obj_loader.h"
#include <vector>
#include "Tools.h"
using namespace std;

struct Ray
{
    //elements
    Vec3f position;
    Vec3f direction;
        
    //methods
    Ray(Vec3f position = Vec3f(0.0, 0.0, 0.0), Vec3f direction = Vec3f(0.0, 0.0, -1.0));
    pair <int, int> getNearestShape(const vector<tinyobj::shape_t> &shapes);
    float getColor(const vector <tinyobj::shape_t> &shapes, Vec3f lightSource);
    
    bool intersect(Vec3f * triangle, Vec3f &result);
    bool intersect_remake(Vec3f * triangle, Vec3f &result);

    bool intersect2(Vec3f * triangleABC, Vec3f &o, Vec3f &w, Vec3f &result);
    

    //solve Mx + Ny = P, where M,N,P are Vec2f each
    bool solveLinear2(float M[], float N[], float P[], float result[]);
};

#endif // RAY_H
