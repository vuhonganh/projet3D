#ifndef TOOLS_H
#define TOOLS_H

#include "Vec3.h"

float brdf_GGX(Vec3f w,Vec3f w0,Vec3f n,float alpha,float f0);

float f_Lambert(float k_d);

float response_color(Vec3f w,Vec3f w0, Vec3f n, float L_w,float alpha,float f0,float k_d);

#endif // TOOLS_H
