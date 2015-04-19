#include "Tools.h"

float brdf_GGX(Vec3f w,Vec3f w0,Vec3f n,float alpha,float f0)
{
    float distr_ggx,fresnel,g,f_s;
    Vec3f w_h=w+w0;
    w_h.normalize();
    //distribution GGX
    float alpha_2=alpha*alpha;
    float nw_h=dot(n,w_h);
    distr_ggx=alpha_2/(M_PI*pow(1+(alpha_2-1)*pow(nw_h,2),2));
    //fresnel terme
    fresnel=f0+(1-f0)*pow(1-fmax(0,nw_h),5);
    //geometric term of GGX
    float nw=dot(n,w);
    
    float nw0=dot(n,w0);
    float g0,g1;
    g0=2*nw0/(nw0+sqrt(alpha_2+(1-alpha_2)*nw0*nw0));
    g1=2*nw/(nw+sqrt(alpha_2+(1-alpha_2)*nw*nw));
    g=g0*g1;
    
    f_s=distr_ggx*fresnel*g/(4*nw*nw0);
    return f_s;
}

float f_Lambert(float k_d){
    float f_L;
    f_L = k_d/M_PI;
    return f_L;
}

float response_color(Vec3f w,Vec3f w0, Vec3f n, float L_w,float alpha,float f0,float k_d)
{
    float L_w0,f_s,f_d,f;
    f_s=brdf_GGX(w,w0,n,alpha,f0);
    f_d=f_Lambert(k_d);
    f=f_d+f_s;
    L_w0=L_w*f*dot(n,w);
    return L_w0;
}