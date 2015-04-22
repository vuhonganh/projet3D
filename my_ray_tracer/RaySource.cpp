#include "RaySource.h"
#define EPS 0.0000001

#define DEBUG(x) cout << #x << " = " << x << endl;

RaySource::RaySource(const vector <Vec3f> &lightSources, Vec3f position, Vec3f lookAtPosition, int resolutionWidth, int resolutionHeight)
{
    this->verticalAngle = acos(-1) * 45/ 180 / 2;
    this->horizontalAngle = acos(-1) * 45 * resolutionWidth / resolutionHeight / 180 / 2;
    this->upVector = Vec3f(0.0f, 1.0f, 0.0f);
    
    this->distanceToScreen = 100;
    this->position = position;
    this->lookAtPosition = lookAtPosition;
    this->resolutionWidth = resolutionWidth;
    this->resolutionHeight = resolutionHeight;
    this->lightSources = lightSources;
}

void RaySource::exportToRGB(const vector<tinyobj::shape_t> &shapes, 
                 const vector <tinyobj::material_t> &materials,
                 BSHNode * bshRoot,
                 unsigned char * rayImage)
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
    
    //generate rays
    for (int yPixel = 0; yPixel < resolutionHeight; ++yPixel)
        for (int xPixel = 0; xPixel < resolutionWidth; ++xPixel)
        {
            float xScreen = 2.0 * xPixel / resolutionWidth - 1;
            float yScreen = 2.0 * yPixel / resolutionHeight - 1;
            Vec3f xVec = xScreen * horizontalVector;
            Vec3f yVec = yScreen * verticalVector;
            Vec3f direction = root + xVec + yVec - position;
            
            //ray calculated
            Ray ray(position, direction);
            
            //calculate average color
            Vec3f color = Vec3f(0.0, 0.0, 0.0);
            for (int iLight = 0; iLight < int(this->lightSources.size()); ++iLight)
            {
                Vec3f cl = ray.getColor(shapes, materials, bshRoot, lightSources[iLight]);
                color += cl;
            }
            color /= int(this->lightSources.size());
            
            
            //set final color
            unsigned int index = 3*(xPixel+yPixel*resolutionWidth);
            rayImage[index] = color[0] * 255;
            rayImage[index+1] = color[1] * 255;
            rayImage[index+2] = color[2] * 255;
        }
}
