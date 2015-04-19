#include "BSHNode.h"
#include <cmath>
#include <iostream>

using namespace std;

BSHNode::BSHNode(const Mesh& m)
{
    split(m.V, m.T);
}

BSHNode::BSHNode()
{
    leftChild   = nullptr;
    rightChild  = nullptr;
}


BSHNode:: BSHNode(const std::vector<Vertex> &V, const std::vector<Triangle> &T)
{
    split(V, T);
}

/*
** 1. Calculer la sphère englobante l'ensemble de triangles
** 2. Séparer en deux régions (deux parites des triangles) 
*/

float min4(float n1, float n2, float n3, float n4)
{
    return min (min( min(n1, n2), n3), n4);
}


float max4(float n1, float n2, float n3, float n4)
{
    return max (max( max(n1, n2), n3), n4);
}


void BSHNode::split(const std::vector<Vertex> &V, const std::vector<Triangle> &T)
{
    //calculer la sphère englobante l'ensemble de triangles
    Vec3f barycentre(0.0f, 0.0f, 0.0f);
    Vec3f barynormal(0.0f, 0.0f, 0.0f);
    //init les xyz min, xyz max
    Vec3f xyzMin(V[0].p[0], V[0].p[1], V[0].p[2]);
    Vec3f xyzMax(V[0].p[0], V[0].p[1], V[0].p[2]);
    for(Triangle t: T)
    {
        Vertex v1 = V[t.v[0]];
        Vertex v2 = V[t.v[1]];
        Vertex v3 = V[t.v[2]];
        //augmente par moyenne de chaque triangle
        barycentre += (v1.p + v2.p + v3.p)/3;
        barynormal += (v1.n + v2.n + v3.n)/3;

        xyzMin = Vec3f(min4(v1.p[0], v2.p[0], v3.p[0], xyzMin[0]), min4(v1.p[1], v2.p[1], v3.p[1], xyzMin[1]), min4(v1.p[2], v2.p[2], v3.p[2], xyzMin[2]));
        xyzMax = Vec3f(max4(v1.p[0], v2.p[0], v3.p[0], xyzMax[0]), max4(v1.p[1], v2.p[1], v3.p[1], xyzMax[1]), max4(v1.p[2], v2.p[2], v3.p[2], xyzMax[2]));
    }

    //prends la moyenne valeur:
    barycentre /= T.size();
    barynormal /= T.size();

    //affectation:
    position = barycentre;
    normal = barynormal;

    //color: (mtn on lui donne une valeur fixe)
    color = Vec3f(0.5f, 0.5f, 0.5f);

    //calculer le rayon
    double max = 0.0;
    for(Triangle t: T)
    {
        Vertex v[3] = {V[t.v[0]],  V[t.v[1]], V[t.v[2]]};
        for(int i = 0; i < 3; i++)
        {
            double dist = (v[i].p - barycentre).length();
            max = (max > dist)? max : dist;
        }
    }
    radius = max;

    //trouver le plan qui sépare les triangles:
    Vec3f delta(xyzMin - xyzMax);
    double maxDirection = abs(delta[0]);
    int indexMaxDir = 0;

    for(int i = 1; i < 3; i++)
    {
        if(maxDirection < abs(delta[i]))
        {
            indexMaxDir = i;
            maxDirection = abs(delta[i]);
        }
    }



    //so sanh composant theo chieu maxDir voi barycentre theo chieu maxDir
    //vi du: max theo Ox, thi so sanh composant x cua barycentre cua triangle vs x cua barycentre general
    std::vector<Triangle> T1;
    std::vector<Triangle> T2;

    T1.reserve(T.size());
    T2.reserve(T.size());

    for(Triangle t: T)
    {
        Vertex v1 = V[t.v[0]];
        Vertex v2 = V[t.v[1]];
        Vertex v3 = V[t.v[2]];

        Vec3f barycentre_t((v1.p + v2.p + v3.p)/3.0);
        Vec3f barynormal_t((v1.n + v2.n + v3.n)/3.0);


        if(barycentre[indexMaxDir] < barycentre_t[indexMaxDir])
            T1.push_back(t);
        else
            T2.push_back(t);

    }

    // std::cout << "ls= " << T1.size() << std::endl;
    // std::cout << "rs= " << T2.size() << std::endl;
    //
    // static int c = 0;
    // ++c;
    // if (c == 100)
    //   exit(0);

    if(T1.size() == 0 || T2.size() == 0)
    {
        leftChild = nullptr;
        rightChild = nullptr;
        return;
    }

    leftChild   = (T1.size() == 0) ? nullptr : new BSHNode(V, T1);
    rightChild  = (T2.size() == 0) ? nullptr : new BSHNode(V, T2);

}

