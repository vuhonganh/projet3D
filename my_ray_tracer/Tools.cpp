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

float ggx(Vec3f camPos, Vec3f source, Vec3f vertex, Vec3f * triangle, float L_w,float alpha,float f0,float k_d)
{
    Vec3f w(source - vertex);
    Vec3f w0(camPos - vertex);
    w.normalize();
    w0.normalize();
    Vec3f n = getNormal(triangle);
    n.normalize();
    
    float L_w0, f_s, f_d, f;
    f_s = brdf_GGX(w, w0, n, alpha, f0);
    f_d = f_Lambert(k_d);
    f = f_d + f_s;
    
    L_w0 = L_w * f * dot(n, w);
    return L_w0;
}

float Lambert (Vec3f source, Vec3f position, Vec3f normal)
{
    Vec3f wi(source - position);
    
    wi /= wi.length();
    return dot(normal, wi); 
}

float blinnPhong(Vec3f camPos, Vec3f source, Vec3f vertex, Vec3f * triangle, float s)
{
    Vec3f wi(source - vertex);
    Vec3f wo(camPos - vertex);
    Vec3f n = getNormal(triangle);
    wi.normalize();
    wo.normalize();
    n.normalize();
    
    Vec3f wh(wi + wo);
    wh.normalize();
    
    return pow(dot(n, wh),s);
}

float toRad(float x)
{
    return acos(-1.0) * x / 180;
}

bool lineCutTrianglePlane(Vec3f * triangle, Vec3f X, Vec3f Y)
{
    Vec3f A = triangle[0];
    Vec3f BA = triangle[1] - A;
    Vec3f CA = triangle[2] - A;

//Vec3f HSLtoRGB(Vec3f hsl)
//{
//    float angle60 = acos(-1.0) * 60 / 180;
//    float C = (1 - fabs(2 * hsl[2] - 1)) * hsl[1];
//    float X = C * (1 - fabs((hsl[0] / angle60) % 2 - 1));
//    float m = hsl[2] - C/2;
    
//    Vec3f rgb;
//    if (hsl[0] >= toRad(0) && hsl[0] < toRad(60))
//        rgb = Vec3f(C, X, 0);
    
//    if (hsl[0] >= toRad(60) && hsl[0] < toRad(120))
//        rgb = Vec3f(X, C, 0);
    
//    if (hsl[0] >= toRad(120) && hsl[0] < toRad(180))
//        rgb = Vec3f(0, C, X);
    
//    if (hsl[0] >= toRad(180) && hsl[0] < toRad(240))
//        rgb = Vec3f(0, X, C);
    
//    if (hsl[0] >= toRad(240) && hsl[0] < toRad(300))
//        rgb = Vec3f(X, 0, C);
    
//    if (hsl[0] >= toRad(300) && hsl[0] < toRad(360))
//        rgb = Vec3f(C, 0, X);
    
//    rgb += Vec3f(m, m, m);
//    return rgb;
//}
   
    //vector normal of the plane ABC:
    Vec3f n = cross(BA, CA);
    n /= n.length();

    //A point P lies in the plane ABC is: (P-A).n = 0
    //with P = tX + (1-t)Y, where t is the length along direction XY
    //il revient a trouver t qui satisfait: t*dot((X-Y),n) = dot((A-Y),n)

    if(fabs(dot(X-Y, n)) > EPS)
    {
        float t = dot(A-Y, n) / (dot(X-Y,n));
        if(t > EPS && t < 1.0 + EPS)
            return true;
    }
    return false;
}

Vec3f getNormal(Vec3f * triangle)
{
    Vec3f A = triangle[0];
    Vec3f BA = triangle[1] - A;
    Vec3f CA = triangle[2] - A;

    //vector normal of the plane ABC:
    Vec3f n = cross(BA, CA);
    n /= n.length();
    return n;
}

float getRandomFloat(float min, float max)
{
    // this  function assumes max > min

    float random = ((float) rand()) / (float) RAND_MAX;

    float range = max - min;
    return (random*range) + min;
}


