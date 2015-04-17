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
    
//    if (b0 < 0 || b1 < 0 || b2 < 0)
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
