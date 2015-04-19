#include "Ray.h"
#define EPS 0.000001

Ray::Ray(Vec3f position, Vec3f direction)
{
    this->position = position;
    this->direction = direction;
}

Vec3f Ray::getBrightness(const vector<tinyobj::shape_t> &shapes, float distanceToScreen, Vec3f lightSource)
{  
    Vec3f bestNormal;
    Vec3f bestIntersection;
    Vec3f bestTriangle[3];
    
    bool flagOK = false;
    float bestDistance = -1;
    int bestShape = 0;
    
    for (size_t s = 0; s < shapes.size(); s++)
        for (size_t f = 0; f < shapes[s].mesh.indices.size() / 3; f++)
        {
            Vec3f triangle[3];
            Vec3f normal[3];
            for (size_t v = 0; v < 3; v++)
            {
                unsigned int index = 3*shapes[s].mesh.indices[3*f+v];
                
                normal[v] = Vec3f(shapes[s].mesh.normals[index],
                                      shapes[s].mesh.normals[index+1],
                                      shapes[s].mesh.normals[index+2]);
                
                triangle[v] = Vec3f(shapes[s].mesh.positions[index],
                                    shapes[s].mesh.positions[index+1],
                                    shapes[s].mesh.positions[index+2]);
            }
            Vec3f intersection;
            if (this->intersect_remake(triangle, intersection))
            {
                flagOK = true;
                
                float distance = (intersection - position).length();
                if (bestDistance < 0 || bestDistance > distance)
                {
                    for (int v = 0; v < 3; ++v)
                        bestTriangle[v] = triangle[v];
                    
                    bestNormal = normal[0];
                    bestDistance = distance;
                    bestIntersection = intersection;
                    bestShape = s;
                }
            }
        }
    
    if (flagOK == false)
    {
        return Vec3f(0.0, 0.0, 0.0);
    }
    else
    {
//        unsigned char grey = 255 * (bestShape + 1) / shapes.size();
//        return Vec3f(grey, grey, grey);
        Vec3f wi = bestIntersection - position;
        Vec3f wo = lightSource - bestIntersection;
        Vec3f n = cross(bestTriangle[1] - bestTriangle[0], bestTriangle[2] - bestTriangle[0]);
        
        wi.normalize();
        wo.normalize();
        n.normalize();
        
        float color = response_color(wi, wo, n, 1.0, 0.5, 0.8, 1.0);
        unsigned char grey = int(255 * color);
        return Vec3f(grey, grey, grey);
    }
}

//TO Hung: need to check if the intersection point lies in the triangle ?
bool Ray::intersect(Vec3f * p, Vec3f &result)
{
    Vec3f o = this->position;
    Vec3f w = this->direction;    
    Vec3f e0 = p[1] - p[0];
    Vec3f e1 = p[2] - p[0];
    Vec3f n = cross(e0, e1) / cross(e0, e1).length();
    Vec3f q = cross(w, e1);
    float a = dot(e0, q);
    
    if (dot(n, w) >= 0 || fabs(a) < EPS)
        return false;
    
    Vec3f s = (o - p[0]) / a;
    Vec3f r = cross(s, e0);
    
//    float b0 = dot(s, q);
//    float b1 = dot(r, w);
//    float b2 = 1 - b0 - b1;
    
//    if (b0 < 0 || b1 < 0 || b2 < 0) //IS IT STILL CORRECT WHILE NOT CHECKING THIS?
//        return false;
    
    float t = dot(e1, r);
    if (t >= 0)
    {
//        result = b0 * p[0] + b1 * p[1] + b2 * p[2];
        result = o + t * w;
        return true;
    }
    
    return false;
}

bool Ray::intersect_remake(Vec3f * triangle, Vec3f &result)
{
    Vec3f o = this->position;
    Vec3f w = this->direction;

    if(intersect2(triangle, o, w, result))
        return true;
    return false;
}

//o is the point initial, w is the direction
bool Ray::intersect2(Vec3f * triangle, Vec3f &o, Vec3f &w, Vec3f &result)
{
    Vec3f A = triangle[0];
    Vec3f BA = triangle[1] - A;
    Vec3f CA = triangle[2] - A;

    //vector normal of the plane ABC:
    Vec3f n = cross(BA, CA);
    n /= n.length();

    //the point lies in the plane is: (P-A).n = 0
    //with X = o + w.t, where t is the distance along direction w
    float t = (dot(A,n) - dot(o,n))/(dot(w,n));
    Vec3f X = o + w*t;
    if (t > EPS)
    {

        float M[] = {dot(BA, BA), dot(BA,CA)};
        float N[] = {dot(BA, CA), dot(CA, CA)};

        Vec3f XA = X - A;
        float P[] = {dot(XA, BA), dot(XA, CA)};
        float resEq[2];
        if(solveLinear2(M, N, P, resEq))
        {
            if( (0 <= resEq[0]) && (resEq[0] <= 1) &&
                (0 <= resEq[1]) && (resEq[1] <= 1) &&
                    resEq[0] + resEq[1] <= 1
              )
            {
                result = X;
                return true;
            }
        }
    }
    return false;
}

//solve Mx + Ny = P, where M,N,P are Vec2f each
bool Ray::solveLinear2(float M[], float N[], float P[], float result[])
{
    float D = M[0]*N[1] - M[1]*N[0];

    if (fabs(D) < EPS)
        return false;
    else
    {
        float Dx = P[0]*N[1] - P[1]*N[0];
        float Dy = M[0]*P[1] - M[1]*P[0];

        result[0] = Dx/D;
        result[1] = Dy/D;
        return true;
    }
}