#include "RaySource.h"
#define EPS 0.0000001

#define DEBUG(x) cout << #x << " = " << x << endl;

RaySource::RaySource(Vec3f position, Vec3f lookAtPosition, int resolutionWidth, int resolutionHeight)
{
    this->verticalAngle = acos(-1) * 30 / 180;
    this->horizontalAngle = acos(-1) * 40 / 180;
    this->distanceToScreen = 200;
    this->upVector = Vec3f(0.0f, 10.0f, 0.0f);
    
    this->position = position;
    this->lookAtPosition = lookAtPosition;
    this->resolutionWidth = resolutionWidth;
    this->resolutionHeight = resolutionHeight;    
}

void RaySource::exportToRGB(const vector<tinyobj::shape_t> &shapes, unsigned char * rayImage)
{
    //find root point
    
    Vec3f lookAtVector = lookAtPosition - position;
    Vec3f root = position + lookAtVector * (distanceToScreen / lookAtVector.length());
    
    float alpha = 0, step = 0.00001;
    for (int i = 0; i < 1000; ++i)
    {
        Vec3f tempVec = upVector + alpha * lookAtVector;
        float d = dot(tempVec, lookAtVector);
        if ((d < -EPS && dot(upVector + float(alpha + step) * lookAtVector, lookAtVector) > d) ||
            (d > EPS && dot(upVector + float(alpha + step) * lookAtVector, lookAtVector) < d))
            alpha += step;
        else
            alpha -= step;
    }
    upVector = upVector + alpha * lookAtVector;
    
    Vec3f verticalVector = upVector * (distanceToScreen * tan(verticalAngle) / upVector.length());
    
    Vec3f rightVector = cross(lookAtVector, upVector);
    Vec3f horizontalVector = rightVector * (distanceToScreen * tan(horizontalAngle) / rightVector.length());
    
    DEBUG(root);
    DEBUG(lookAtVector);
    DEBUG(verticalVector);
    DEBUG(horizontalVector);
    
    //generate rays
    for (int yPixel = 0; yPixel < resolutionHeight; ++yPixel)
        for (int xPixel = 0; xPixel < resolutionWidth; ++xPixel)
        {
            float xScreen = 2.0 * xPixel / resolutionWidth - 1;
            float yScreen = 2.0 * yPixel / resolutionHeight - 1;
            Vec3f xVec = xScreen * horizontalVector;
            Vec3f yVec = yScreen * verticalVector;
            Vec3f direction = root + xVec + yVec - position;
            
            Ray ray(position, direction);
            Vec3f brightness = ray.getBrightness(shapes, distanceToScreen);
            
            unsigned int index = 3*(xPixel+yPixel*resolutionWidth);

            rayImage[index] = brightness[0] * 255;
            rayImage[index+1] = brightness[1] * 255;
            rayImage[index+2] = brightness[2] * 255;
        }
}