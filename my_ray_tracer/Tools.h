#ifndef TOOLS_H
#define TOOLS_H

#include "Vec3.h"
#include <vector>
#include "tiny_obj_loader.h"
#define EPS 0.000001
using namespace std;

float brdf_GGX(Vec3f w,Vec3f w0,Vec3f n,float alpha,float f0);

float f_Lambert(float k_d);

float response_color(Vec3f vertex, Vec3f source, Vec3f camPos, Vec3f n, float L_w,float alpha,float f0,float k_d);

float Lambert (Vec3f source, Vec3f position, Vec3f normal);

float BlinnPhong(Vec3f vertex, Vec3f source, Vec3f camPos, Vec3f normal, float s);

bool lineCutTrianglePlane(Vec3f * triangle, Vec3f X, Vec3f Y);

//Vec3f RGBtoHSV(Vec3f rgb);

#endif // TOOLS_H
