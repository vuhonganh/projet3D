#ifndef RAY_H
#define RAY_H

#include "Vec3.h"
#include "tiny_obj_loader.h"
#include <vector>
#include "Tools.h"
#include "BSHNode.h"
using namespace std;

struct Ray
{
    //elements
    Vec3f position;
    Vec3f direction;
        
    //methods
    Ray(Vec3f position = Vec3f(0.0, 0.0, 0.0), Vec3f direction = Vec3f(0.0, 0.0, -1.0));
    
    void getNearestTriangleByBSHNode(const vector <tinyobj::shape_t> &shapes,
                                            BSHNode * bshRoot,
                                            pair <int, int> exceptionTriangle,
                                            pair <int, int> &resultIndex,
                                            float &resultDistance);
    
    pair <int, int> getNearestTriangle(const vector <tinyobj::shape_t> &shapes,
                                       BSHNode * node,
                                       pair <int, int> exceptionTriangle);
    
    Vec3f getColor(const vector <tinyobj::shape_t> &shapes, 
                   const vector <tinyobj::material_t> &materials, 
                   BSHNode * bshRoot,
                   Vec3f lightSource);
    
    bool intersect(Vec3f * triangle, Vec3f &result);
    bool intersect_remake(Vec3f * triangle, Vec3f &result);
    bool intersect2(Vec3f * triangleABC, Vec3f &o, Vec3f &w, Vec3f &result);
    bool intersect_sphere(Vec3f center, float radius);

    //solve Mx + Ny = P, where M,N,P are Vec2f each
    bool solveLinear2(float M[], float N[], float P[], float result[]);
};

#endif // RAY_H
