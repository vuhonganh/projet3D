#include "Ray.h"

static int NUMBER_OF_RAYS = 100;
static int MAX_DEPTH = 2;
static float EPS = 0.0001;

Ray::Ray(Vec3f position, Vec3f direction, BSHNode * bshRoot, int depth, pair <int, int> exceptionTriangle, bool dbg)
{
    this->position = position;
    this->exceptionTriangle = exceptionTriangle;
    this->direction = direction;
    this->direction.normalize();
    this->DBG = dbg;
    this->depth = depth;
    this->bshRoot = bshRoot;
}

//get nearest triangle except exceptionTriangle
pair <int, int> Ray::getNearestTriangle_KDTree(const vector <tinyobj::shape_t> &shapes)
{
    pair <int, int> resultIndex = make_pair(-1, -1);
    float resultDistance = -1;

    this->getNearestTriangleByBSHNode(shapes, bshRoot, resultIndex, resultDistance);

    return resultIndex;
}

void Ray::getNearestTriangleByBSHNode(const vector <tinyobj::shape_t> &shapes,
                                      BSHNode * node,
                                      pair <int, int> &resultIndex,
                                      float &resultDistance)
{
    if (this->intersect_sphere(node->center, node->radius) == false)
        return;

    if (node->leftChild == NULL && node->rightChild == NULL)
    {
        for (pair <int, int> index: node->triangleIds)
        {
            if (index == exceptionTriangle) continue;

            Vec3f triangle[3];
            getTrianglePositionFromShape(shapes, index.first, index.second, triangle);

            Vec3f intersection;
            if (this->intersect_remake(triangle, intersection))
            {
                float dist = (this->position - intersection).length();
                if (resultDistance < 0 || dist < resultDistance)
                {
                    resultDistance = dist;
                    resultIndex = index;
                }
            }
        }
    }
    else
    {
        if (node->leftChild != NULL)
            this->getNearestTriangleByBSHNode(shapes, node->leftChild, resultIndex, resultDistance);

        if (node->rightChild != NULL)
            this->getNearestTriangleByBSHNode(shapes, node->rightChild, resultIndex, resultDistance);
    }
}

////brute-force
pair <int, int> Ray::getNearestTriangle_BruteForce(const vector <tinyobj::shape_t> &shapes)
{
    bool flagOK = false;
    float bestDistance = -1;
    pair <int, int> result;

    //try all triangles for finding the nearest one
    for (size_t s = 0; s < shapes.size(); s++)
        for (size_t f = 0; f < shapes[s].mesh.indices.size() / 3; f++)
        {
            if (int(s) == exceptionTriangle.first && int(f) == exceptionTriangle.second) continue;

            //get triangle from messs
            Vec3f triangle[3];
            getTrianglePositionFromShape(shapes, s, f, triangle);

            //get intersection
            Vec3f intersection;
            if (this->intersect_remake(triangle, intersection))
            {
                DBG && cout << "cut " << s << ' ' << f << endl;
                
                //there is an interection
                flagOK = true;
                float distance = (intersection - position).length();

                //check if the ray meets intersection before lightSource
                if (bestDistance < 0 || bestDistance > distance)
                {
                    //update minimum distance
                    bestDistance = distance;
                    result = make_pair(s, f);
                }
            }
        }

    //check if the ray meets at least one intersection
    if (flagOK == false)
        return make_pair(-1, -1); //if false => not found
    else
        return result; //if true
}

pair <int, int> Ray::getIntersectTriangle(const vector <tinyobj::shape_t> &shapes,
                         Vec3f * triangle)
{
    pair <int, int> shapeId = this->getNearestTriangle_KDTree(shapes);
//    pair <int, int> shapeId = this->getNearestTriangle_BruteForce(shapes);
    if (shapeId.first == -1) return make_pair(-1, -1);

    getTrianglePositionFromShape(shapes, shapeId.first, shapeId.second, triangle);
    return shapeId;
}

bool Ray::canReach(Vec3f point, const vector <tinyobj::shape_t> &shapes)
{
    Vec3f triangle[3];
    pair <int, int> shapeId = this->getIntersectTriangle(shapes, triangle);

    if (shapeId.first != -1)
    {
        Vec3f intersection;
        this->intersect_remake(triangle, intersection);

        //if the triangle is in front of the light source => cancel
        if ((this->position - intersection).length() < (this->position - point).length())
            return false;
        else
            return true;
    }
    else
        return true;
}

Vec3f Ray::getColor(const vector <tinyobj::shape_t> &shapes,
               const vector <tinyobj::material_t> &materials,
               Vec3f lightSource)
{       
    if (DBG) cout << "[getColor] depth = " << depth << endl;
    
    //get triangle that intersect the ray
    Vec3f triangle[3];
    pair <int, int> triangleId = this->getIntersectTriangle(shapes, triangle);
    
    DBG && cout << "\t[triangleId] " << triangleId.first << ' ' << triangleId.second << endl;
    
    if (triangleId.first == -1)
    {
        DBG && cout << "\t[return] no intersection" << endl;
        return Vec3f(0.0, 0.0, 0.0);
    }
    
    Vec3f reversedDirection = this->direction * -1;
    if (dot(reversedDirection, getNormalwithRayComes(triangle, this->direction)) < 0)
    {
        DBG && cout << "[return] " << endl;
        return Vec3f(0.0, 0.0, 0.0);
    }
    
    //check if position and lightsource are in different sides of the triangle    
    Vec3f intersection;
    Vec3f color_direct;
    
    unsigned int iMaterial = shapes[triangleId.first].mesh.material_ids[triangleId.second];
    if (lineCutTrianglePlane(triangle, this->direction, this->position, lightSource))
    {
        DBG && cout << "\t[message] lineCutTrianglePlane" << endl;
        color_direct = Vec3f(0.0, 0.0, 0.0);
    }
    else
    {
        //calculate intersection
        this->intersect_remake(triangle, intersection);

        //check reflected ray
        Ray reflectedRay(intersection, lightSource - intersection, bshRoot, 0, triangleId);
        if (reflectedRay.canReach(lightSource, shapes) == false)
        {
            DBG && cout << "\t[message] reflected ray cannot reach lightsource" << endl;
            color_direct = Vec3f(0.0, 0.0, 0.0);
        }
        else
        {
            //calculate color_direct
            float radian_direct = ggx(this->position, lightSource, intersection, triangle, 1.0, 0.8, 0.8, 2.0);
            
//            color_direct = Vec3f(   (materials[iMaterial].diffuse[0] + materials[iMaterial].specular[0]) * radian_direct, 
//                                    (materials[iMaterial].diffuse[1] + materials[iMaterial].specular[1]) * radian_direct,
//                                    (materials[iMaterial].diffuse[2] + materials[iMaterial].specular[2]) * radian_direct);
            
            color_direct = Vec3f(   (materials[iMaterial].diffuse[0]) * radian_direct, 
                                    (materials[iMaterial].diffuse[1]) * radian_direct,
                                    (materials[iMaterial].diffuse[2]) * radian_direct);
            
            DBG && cout << "\t[color] " << color_direct << endl;
        }
    }

    Vec3f color_indirect(0.0, 0.0, 0.0);
    int counter = 0;
    if (depth < MAX_DEPTH)
    {        
        for (int iRay = 0; iRay < NUMBER_OF_RAYS; ++iRay)
        {
            Ray ray = this->getRandomRay_Sphere(intersection, triangle, depth + 1, triangleId);
            
//            if (triangleId.first == 4)
//                ray = this->getMirrorRay(intersection, triangle, depth + 1, triangleId);
            
//            Ray ray = this->getRandomRay_Sphere(intersection, triangle, depth + 1, triangleId);
//            Ray ray = this->getInConeRay(intersection, triangle, depth + 1, triangleId);
//            Ray ray = this->getUniformRay_Plane(intersection, triangle, depth + 1, triangleId, iRay, NUMBER_OF_RAYS);
//            Ray ray = this->getMirrorRay(intersection, triangle, depth + 1, triangleId);

            Vec3f color = ray.getColor(shapes, materials, lightSource);

            float cos_theta = dot(ray.direction, getNormalwithRayComes(triangle, this->direction));
            Vec3f w = lightSource - intersection;
            Vec3f w0 = this->position - intersection;
            Vec3f n = getNormalwithRayComes(triangle, this->direction);
            w.normalize();
            w0.normalize();
            n.normalize();
            float f_s = brdf_GGX(w, w0, n, 0.8, 0.8);
            float f_d = f_Lambert(2.0);
//            float f_d = 0;

            color_indirect += color * (f_s + f_d) * fabs(cos_theta);
//            color_indirect += color * (f_s + f_d);

            counter++;
        }

        if (counter > 0)
            color_indirect /= counter;
        
        DBG && cout << "\t[color_indirect] = " << color_indirect << endl;
    }
    
    DBG && cout << "\t[counter] " << counter << endl;

    DBG && cout << "\t[color] " << color_direct << endl;
//    color_direct = (color_direct * 0.5) + (color_indirect * 0.5);
    
    color_direct += (1.0f * color_indirect);
    for (int i = 0; i < 3; ++i)
        color_direct[i] = min(color_direct[i], 1.0f);
    
    if (this->depth == 1)
        DBG && cout << "final color = " << color_direct << endl;
    
//    return color_direct + Vec3f(materials[iMaterial].ambient[0],
//                                materials[iMaterial].ambient[1],
//                                materials[iMaterial].ambient[2]);
    
    return color_direct;
}

//TO Hung: need to check if the intersection point lies in the triangle ?
bool Ray::intersect(Vec3f * p, Vec3f &result)
{
    Vec3f o = this->position;
    Vec3f w = this->direction;
    Vec3f e0 = p[1] - p[0];
    Vec3f e1 = p[2] - p[0];
    Vec3f n = cross(e0, e1) / cross(e0, e1).length();
    Vec3f q = cross(w, e1);
    float a = dot(e0, q);

    if (dot(n, w) >= 0 || fabs(a) < EPS)
        return false;

    Vec3f s = (o - p[0]) / a;
    Vec3f r = cross(s, e0);

    float t = dot(e1, r);
    if (t >= 0)
    {
        result = o + t * w;
        return true;
    }

    return false;
}

bool Ray::intersect_remake(Vec3f * triangle, Vec3f &result)
{
    Vec3f o = this->position;
    Vec3f w = this->direction;

    if(intersect2(triangle, o, w, result))
        return true;
    return false;
}

bool check(Vec3f A, Vec3f B, Vec3f C, Vec3f X)
{
    Vec3f ab = B - A;
    Vec3f ac = C - A;
    Vec3f ax = X - A;
    ab.normalize();
    ac.normalize();
    ax.normalize();
    
    if (acos(dot(ab, ax)) < acos(dot(ab, ac)) && acos(dot(ac, ax)) < acos(dot(ab, ac)))
        return true;
    else
        return false;
}

//o is the point initial, w is the direction
bool Ray::intersect2(Vec3f * triangle, Vec3f &o, Vec3f &w, Vec3f &result)
{
    Vec3f A = triangle[0];
    Vec3f B = triangle[1];
    Vec3f C = triangle[2];
    Vec3f BA = triangle[1] - A;
    Vec3f CA = triangle[2] - A;

    //vector normal of the plane ABC:
    Vec3f n = cross(BA, CA);
    n /= n.length();
    
//    if (dot(n, w) > EPS)
//        n *= -1;

    //the point lies in the plane is: (X-A).n = 0
    //with X = o + w.t, where t is the distance along direction w

//    if(dot(w,n) < EPS)
//        return false;

    Vec3f Ao = A-o;

    float t = (dot(Ao, n)) / (dot(w,n));
    Vec3f X = o + w*t;
    
    if (t > 0)
    {

        float M[] = {dot(BA, BA), dot(BA,CA)};
        float N[] = {dot(BA, CA), dot(CA, CA)};

        Vec3f XA = X - A;
        float P[] = {dot(XA, BA), dot(XA, CA)};
        float resEq[2];
        if(solveLinear2(M, N, P, resEq))
        {
            if( (0 <= resEq[0]) && (resEq[0] <= 1) &&
                (0 <= resEq[1]) && (resEq[1] <= 1) &&
                    resEq[0] + resEq[1] <= 1
              )
            {
                result = X;
                return true;
            }
        }
    }
    return false;
}

//solve Mx + Ny = P, where M,N,P are Vec2f each
bool Ray::solveLinear2(float M[], float N[], float P[], float result[])
{
    float D = M[0]*N[1] - M[1]*N[0];

    if (fabs(D) < EPS)
        return false;
    else
    {
        float Dx = P[0]*N[1] - P[1]*N[0];
        float Dy = M[0]*P[1] - M[1]*P[0];

        result[0] = Dx/D;
        result[1] = Dy/D;
        return true;
    }
}

//check if ray intersect with a sphere
bool Ray::intersect_sphere(Vec3f center, float radius)
{
    Vec3f oc = this->position - center;
    float A = dot(this->direction, this->direction);
    float B = 2 * dot(oc, this->direction);
    float C = dot(oc, oc) - radius * radius;

    float delta = B * B - 4 * A * C;

    if (delta < -EPS) return false;
    if (delta >= -EPS && delta <= EPS) return true;

    float t1 = (-B + sqrt(delta))/(2*A);
    float t2 = (-B - sqrt(delta))/(2*A);

    //cut from outside, choose the smaller one (i.e. the nearer)
    if((t1 >= -EPS) && (t2 >= -EPS)) return true;
    else
        if (t1 * t2 < -EPS) //cut from inside: one negative, one positive, choose the positive one (not sure)
            return true;

    return false;
}

Ray Ray::getRandomRay_Sphere(Vec3f intersection, Vec3f * triangle, int depth, pair <int, int> exceptionTriangle)
{
    Vec3f en = getNormalwithRayComes(triangle, this->direction);

    //find a unit vector in the plane
    Vec3f ex(triangle[0] - intersection);
    ex.normalize();

    //another unit vector in the plane to forme a local coordiante
    Vec3f ey = cross(en, ex);
    ey.normalize();
    
    float angleN = getRandomFloat(0.0, acos(-1.0) / 2);
    float angleXY = getRandomFloat(0.0, 2 * acos(-1.0));
    
    Vec3f dirOut((ex * cos(angleXY) * sin(angleN)) +
                 (ey * sin(angleXY) * sin(angleN)) +
                 (en * cos(angleN)));
    
    dirOut.normalize();
    
    return Ray(intersection, dirOut, bshRoot, depth, exceptionTriangle, this->DBG);
}

Ray Ray::getInConeRay(Vec3f intersection, Vec3f * triangle, int depth, pair <int, int> exceptionTriangle)
{
    Vec3f n = getNormalwithRayComes(triangle, this->direction);
    //find a unit vector in the plane
    Vec3f ex(triangle[0] - intersection);
    ex /= ex.length();

    //another unit vector in the plane to forme a local coordiante
    Vec3f ey = cross(n, ex);
    Vec3f wi(this->position - intersection);
    wi /= wi.length();

    float cosTheta_i = dot(wi, n);
    if(cosTheta_i < -EPS)
        cout << "ERROR in getInConeRaysOut: l'angle theta_i est plus grande que 90, something wrong with the direction" << endl;

    //find a eOnPlane vector unit (which lies on plane)
    float xComponent     = getRandomFloat(-1.0, 1.0);
    float yComponent     = getRandomFloat(-1.0, 1.0);
    Vec3f eOnPlane(xComponent * ex + yComponent * ey);
    eOnPlane /= eOnPlane.length();

    //to ensure the direction Out lies inside the cone
    //dirOut = costheta_i * normal + sintheta_i * eOnPlane
    Vec3f dirOut(cosTheta_i * n + sqrt(1 - cosTheta_i*cosTheta_i) * eOnPlane);
    dirOut /= dirOut.length();

    return Ray(intersection, dirOut, bshRoot, depth, exceptionTriangle);
}

Ray Ray::getRay(Vec3f intersection, Vec3f * triangle, int depth, pair <int, int> exceptionTriangle, float angleN, float angleXY)
{
    Vec3f en = getNormalwithRayComes(triangle, this->direction);

    //find a unit vector in the plane
    Vec3f ex(triangle[0] - intersection);
    ex.normalize();

    //another unit vector in the plane to forme a local coordiante
    Vec3f ey = cross(en, ex);
    ey.normalize();
    
    Vec3f dirOut((ex * cos(angleXY) * sin(angleN)) +
                 (ey * sin(angleXY) * sin(angleN)) +
                 (en * cos(angleN)));
    
    dirOut.normalize();
    
    return Ray(intersection, dirOut, bshRoot, depth, exceptionTriangle, this->DBG);    
}

Ray Ray::getUniformRay_Plane(Vec3f intersection, Vec3f * triangle, int depth, pair <int, int> exceptionTriangle, int rayId, int nRay)
{
    float unitAngle = acos(-1.0) / 2 / nRay;
    Vec3f n = getNormalwithRayComes(triangle, this->direction);
    Vec3f ez = cross(this->direction, n);
    Vec3f ex = cross(n, ez);
    
    float alpha = unitAngle * (rayId + 1);
    Vec3f dirOut = (cos(alpha) * n + sin(alpha) * ex);
    dirOut.normalize();
    
    return Ray(intersection, dirOut, bshRoot, depth, exceptionTriangle, this->DBG);
}

Ray Ray::getMirrorRay(Vec3f intersection, Vec3f * triangle, int depth, pair <int, int> exceptionTriangle)
{
    Vec3f n = getNormalwithRayComes(triangle, this->direction);
    float alpha = acos(dot(-this->direction, n));
    Vec3f ez = cross(this->direction, n);
    Vec3f ex = cross(n, ez);
    
    Vec3f dirOut = cos(alpha) * n + sin(alpha) * ex;
    dirOut.normalize();
    
    return Ray(intersection, dirOut, bshRoot, depth, exceptionTriangle, this->DBG);
}
