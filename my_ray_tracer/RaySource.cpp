#include "RaySource.h"
#define EPS 0.0000001

#define DEBUG(x) cout << #x << " = " << x << endl;

RaySource::RaySource(Vec3f position, Vec3f lookAtPosition, Vec3f upVector,
       float distanceToScreen, float verticalAngle, float horizontalAngle,
       int resolutionWidth, int resolutionHeight)
{
    this->position = position;
    this->lookAtPosition = lookAtPosition;
    this->upVector = upVector;
    this->verticalAngle = verticalAngle;
    this->horizontalAngle = horizontalAngle;
    this->distanceToScreen = distanceToScreen;
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
        cout << i << ' ' << alpha << endl;
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
            
            if (xPixel == 0)
            {
//                DEBUG(xScreen);
//                DEBUG(yScreen);
//                DEBUG(xVec);
//                DEBUG(yVec);
//                cout << "(" << yPixel << ", " << xPixel << ") ";
//                cout << xVec + yVec << endl;
            }
            
            Ray ray(position, direction);
            Vec3f brightness = ray.getBrightness(shapes, distanceToScreen);
            
            unsigned int index = 3*(xPixel+yPixel*resolutionWidth);
//            rayImage[index] = rayImage[index+1] = rayImage[index+2] = grey;
            rayImage[index] = brightness[0] * 255;
            rayImage[index+1] = brightness[1] * 255;
            rayImage[index+2] = brightness[2] * 255;
//            if (grey > 0) cout << xPixel << ' ' << yPixel << ' ' << grey << endl;
        }
}