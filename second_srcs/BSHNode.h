#ifndef BSHNODE_H
#define BSHNODE_H

#include "Vec3.h"
#include "tiny_obj_loader.h"
#include <vector>
#include <cmath>
#include <iostream>
#define TRIANGLES_PER_LEAF 5

using namespace tinyobj;

struct BSHNode
{
    //elements
    vector <pair <int, int> > triangleIds;
    BSHNode * leftChild;
    BSHNode * rightChild;
    
    Vec3f center;
    Vec3f normal;
    float radius;
    
    int splitDirection;
    float splitValue;
    
    //methods
    BSHNode(const vector <shape_t> &shapes, const vector <pair <int, int> > &triangleIds);
    ~BSHNode();
};

#endif
