#ifndef TOOLS_H
#define TOOLS_H

#include "Vec3.h"
#include <vector>
#include <cmath>
#include "tiny_obj_loader.h"


using namespace std;

float brdf_GGX(Vec3f w,Vec3f w0,Vec3f n,float alpha,float f0);

float f_Lambert(float k_d);

float rcolor(Vec3f vertex, Vec3f source, Vec3f camPos, Vec3f n, float L_w,float alpha,float f0,float k_d);

inline float brdf(Vec3f wi, Vec3f wo, Vec3f n,float alpha,float f0,float k_d){
    return f_Lambert(k_d)+brdf_GGX(wi,wo,n,alpha,f0);}

//theta ~ arccos(v) v~Uniform[0,1)
inline float randomTheta(){
    float random = ((float) rand()) / (float) RAND_MAX;
        return acos(random);
}

//phi ~ 2*Pi*u, u~ Uniform[0,1)
inline float randomPhi(){
    float random = ((float) rand()) / (float) RAND_MAX;
        return 2*M_PI*random;
}

// generate randomly a new ray
//inline Vec3f newRandomRay(const Vec3f &normal, float &phi,float &theta){

//}

float ggx(Vec3f camPos, Vec3f source, Vec3f vertex, Vec3f * triangle, float L_w,float alpha,float f0,float k_d);

float Lambert (Vec3f source, Vec3f position, Vec3f normal);

float blinnPhong(Vec3f camPos, Vec3f source, Vec3f vertex, Vec3f * triangle, float s);
//float BlinnPhong(Vec3f vertex, Vec3f source, Vec3f camPos, Vec3f normal, float s);

bool lineCutTrianglePlane(Vec3f * triangle, Vec3f X, Vec3f Y);

Vec3f getNormal(Vec3f * triangle);

float getRandomFloat(float min, float max);

Vec3f getNormalwithRayComes(Vec3f * triangle, Vec3f rayDirection);


#endif // TOOLS_H
