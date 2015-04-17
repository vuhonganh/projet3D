#include "LightSource.h"

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
