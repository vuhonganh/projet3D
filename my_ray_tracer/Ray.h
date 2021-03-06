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
    int depth;
    BSHNode * bshRoot;
    bool DBG;
    
    pair <int, int> exceptionTriangle;
        
    //methods
    Ray(Vec3f position, Vec3f direction, BSHNode * bshRoot, int depth, pair <int, int> exceptionTriangle, bool dbg = false);
    
    void getNearestTriangleByBSHNode(const vector <tinyobj::shape_t> &shapes,
                                     BSHNode * node,
                                     pair <int, int> &resultIndex,
                                     float &resultDistance);
    
    pair <int, int> getIntersectTriangle(const vector <tinyobj::shape_t> &shapes, Vec3f * triangle);
    pair <int, int> getNearestTriangle_KDTree(const vector <tinyobj::shape_t> &shapes);
    pair <int, int> getNearestTriangle_BruteForce(const vector <tinyobj::shape_t> &shapes);
    
    Vec3f getColor(const vector <tinyobj::shape_t> &shapes, 
                   const vector <tinyobj::material_t> &materials, 
                   Vec3f lightSource);
    
    //a partir de la rayon qui vient, l'intersection avec la triangle
    //generer nbRays nouvelles rayons qui sorte de facon aleatoire
    
    Ray getRandomRay_Sphere(Vec3f intersection, Vec3f * triangle, int depth, pair <int, int> exceptionTriangle);
    Ray getInConeRay(Vec3f intersection, Vec3f * triangle, int depth, pair <int, int> exceptionTriangle);
    Ray getUniformRay_Plane(Vec3f intersection, Vec3f * triangle, int depth, pair <int, int> exceptionTriangle, int rayId, int nRay);
    Ray getRay(Vec3f intersection, Vec3f * triangle, int depth, pair <int, int> exceptionTriangle, float angleNormal, float anglePlane);
    Ray getMirrorRay(Vec3f intersection, Vec3f * triangle, int depth, pair <int, int> exceptionTriangle);
    
    bool intersect(Vec3f * triangle, Vec3f &result);
    bool intersect_remake(Vec3f * triangle, Vec3f &result);
    bool intersect2(Vec3f * triangleABC, Vec3f &o, Vec3f &w, Vec3f &result);
    bool intersect_sphere(Vec3f center, float radius);

    //solve Mx + Ny = P, where M,N,P are Vec2f each
    bool solveLinear2(float M[], float N[], float P[], float result[]);
    
    bool canReach(Vec3f point, const vector <tinyobj::shape_t> &shapes);
};

#endif // RAY_H
