#ifndef BSHNODE_H
#define BSHNODE_H

#include "Mesh.h"
#include "Vec3.h"
#include <vector>

class BSHNode
{
 public:
  BSHNode(const Mesh& m);
  BSHNode(const std::vector<Vertex> &V, const std::vector<Triangle> &T);
  void split(const std::vector<Vertex> &V, const std::vector<Triangle> &T);
  BSHNode();
	
  static const int TRIANGLES_PER_LEAF = 3;

  //ci-dessous sont des valeurs moyennes:
  Vec3f position;//position of the center of sphere
  Vec3f normal;//a normal vector average
  Vec3f color;

  float radius;//radius of the sphere

  BSHNode * leftChild;
  BSHNode * rightChild;
  
};

#endif
