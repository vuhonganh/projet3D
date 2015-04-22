#include "BSHNode.h"

using namespace std;
using namespace tinyobj;

float min3(float a, float b, float c)
{
    return min(min(a, b), c);
}

float min4(float a, float b, float c, float d)
{
    return min(min(a, b), min(c, d));
}

float max3(float a, float b, float c)
{
    return max(max(a, b), c);
}

float max4(float a, float b, float c, float d)
{
    return max(max(a, b), max(c, d));
}

int nodeCounter = 0;

BSHNode::BSHNode(const vector <shape_t> &shapes, const vector <pair <int, int> > &triangleIds)
{   
    nodeCounter++;
    cout << "nodeCounter = " << nodeCounter << endl;
//    cout << "size = " << triangleIds.size() << endl;
    
    this->triangleIds.clear();
    this->leftChild = NULL;
    this->rightChild = NULL;
    
    //calculate center and normal
    this->center = Vec3f(0.0, 0.0, 0.0);
    this->normal = Vec3f(0.0, 0.0, 0.0);
    this->radius = 0;
    Vec3f xyzMin, xyzMax;
    bool flag_xyz_MinMax = false;
    
    for (pair <int, int> index: triangleIds)
    {
        //calculate trianglePos
        Vec3f pos[3];
        getTrianglePositionFromShape(shapes, index.first, index.second, pos);
        this->center += (pos[0] + pos[1] + pos[2]) / 3;
        
        //calculate triangleNor
        Vec3f nor[3];
        getTriangleNormalFromShape(shapes, index.first, index.second, nor);
        this->normal += (nor[0] + nor[1] + nor[2]) / 3;
        
        //update xyz_MinMax
        if (flag_xyz_MinMax == false)
        {
            flag_xyz_MinMax = true;
            xyzMin = xyzMax = (pos[0] + pos[1] + pos[2]) / 3;
        }
        else
        {
            Vec3f average = (pos[0] + pos[1] + pos[2]) / 3;
            xyzMin = Vec3f( min(average[0], xyzMin[0]),
                            min(average[1], xyzMin[1]),
                            min(average[2], xyzMin[2]) );
            
            xyzMax = Vec3f( max(average[0], xyzMax[0]),
                            max(average[1], xyzMax[1]),
                            max(average[2], xyzMax[2]) );
        }
    }
    
    this->center /= int(triangleIds.size());
    this->normal /= int(triangleIds.size());
    
    //calculate radius
    this->radius = -1;
    for (pair <int, int> index: triangleIds)
    {
        Vec3f pos[3];
        getTrianglePositionFromShape(shapes, index.first, index.second, pos);
        
        for (int i = 0; i < 3; ++i)
        {
            float dist = (this->center - pos[i]).length();
            if (this->radius < -1 || this->radius < dist)
                this->radius = dist;
        }
    }
    
    //check split
    if (triangleIds.size() <= TRIANGLES_PER_LEAF)
    {
        this->triangleIds = triangleIds;
        return;
    }
    
    //split
    Vec3f delta(xyzMax - xyzMin);
    this->splitDirection = -1;
    this->splitValue = 0;
    
    for (int i = 0; i < 3; ++i)
        if (this->splitDirection < 0 || delta[i] > delta[this->splitDirection])
        {
            this->splitDirection = i;
            this->splitValue = xyzMin[i] + delta[i] / 2;
        }
    
//    cout << "min = " << xyzMin[this->splitDirection] << endl;
//    cout << "max = " << xyzMax[this->splitDirection] << endl;
//    cout << "splitValue = " << this->splitValue << endl;
//    cout << endl;
    
    vector <pair <int, int> > leftIds;
    vector <pair <int, int> > rightIds;
    for (pair <int, int> index: triangleIds)
    {
        Vec3f triangle[3];
        getTrianglePositionFromShape(shapes, index.first, index.second, triangle);
        
        Vec3f triangeCenter = (triangle[0] + triangle[1] + triangle[2]) / 3;
        
        if (triangeCenter[this->splitDirection] <= this->splitValue)
            leftIds.push_back(index);
        else
            rightIds.push_back(index);
    }
    
    if (leftIds.empty() == false) 
        this->leftChild = new BSHNode(shapes, leftIds);
    
    if (rightIds.empty() == false)
        this->rightChild = new BSHNode(shapes, rightIds);
}

BSHNode::~BSHNode()
{
    if (this->leftChild != NULL) delete this->leftChild;
    if (this->rightChild != NULL) delete this->rightChild;
}