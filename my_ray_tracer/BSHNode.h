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

  //ci-dessous sont des valeurs moyennes:
  Vec3f position;
  Vec3f normal;
  Vec3f color;

  float radius;

  BSHNode * leftChild;
  BSHNode * rightChild;
  
};

#endif
