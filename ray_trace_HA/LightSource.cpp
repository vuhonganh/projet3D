#include "LightSource.h"
#define EPS 0.00000001

Direction::Direction(float angleXZ, float angleY)
{
    this->angleXZ = angleXZ;
    this->angleY = angleY;
}

Ray::Ray(Vec3f position, Direction direction)
{
    this->position = position;
    this->direction = direction;
}


Ray::Ray(Vec3f position, Vec3f vectorDir)
{
    this->position = position;
    this->vectorDir = vectorDir;
}

LightSource::LightSource(int type, Vec3f position)
{
    this->position = position;
    
    if (type == LIGHT_SOURCE_NORMAL)
    {
        int nStep = 36;
        float angleStep = acos(-1.0) / nStep;
        for (float angleXZ = 0; angleXZ < acos(-1.0) * 2; angleXZ += angleStep)
            for (float angleY = 0; angleY < acos(-1.0) * 2; angleY += angleStep)
            {
                Ray ray(this->position, Direction(angleXZ, angleY));
                this->rays.push_back(ray);
            }
    }
}

Vec3f Direction::toVector()
{
    return Vec3f(sin(this->angleY) * cos(this->angleXZ),
                 cos(this->angleY),
                 sin(this->angleY) * sin(this->angleXZ)
                 );
}

//TO Hung: need to check if the intersection point lies in the triangle ?
bool Ray::intersect(Vec3f * p, Vec3f &result)
{
    Vec3f o = this->position;
    Vec3f w = this->direction.toVector();    
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
    Vec3f w = this->direction.toVector();

    if(intersect2(triangle, o, w, result))
        return true;
    return false;
}

//o is the point initial, w is the direction
bool Ray::intersect2(Vec3f * triangleABC, Vec3f &o, Vec3f &w, Vec3f &result)
{
    Vec3f A = triangleABC[0];
    Vec3f BA = triangleABC[1] - A;
    Vec3f CA = triangleABC[2] - A;

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

//the intersection point presented by: o + w*t
//it satisfies the equation of the sphere surface: (o + w*t - c)(o + w*t - c) = r^2
bool Ray::intersect_Sphere(Vec3f &o, Vec3f &w, Vec3f &c, float &r, Vec3f &result)
{
    Vec3f oc = o - c;

    //problem leads to solve second degree equation of form: At^2 + Bt + C = 0
    float A = dot(w,w);
    float B = 2*dot(oc, w);
    float C = dot(oc, oc) - r*r;

    float delta = B*B - 4*A*C;

    if(delta < 0)
        return false;
    else if(delta == 0)
    {
        float t = -B/(2*A);
        result = o + w*t;
        return true;
    }
    else
    {
        float t1 = (-B + sqrt(delta))/(2*A);
        float t2 = (-B - sqrt(delta))/(2*A);

        //cut from outside, choose the smaller one (i.e. the nearer)
        if( (t1 >= 0) && (t2 >= 0) )
        {
            result = o + w*min(t1, t2);
            return true;
        }
        else if(t1 * t2 < 0) //cut from inside: one negative, one positive, choose the positive one (not sure)
        {
            result = o + w*max(t1, t2);
            return true;
        }
        else
            return false;
    }
}











