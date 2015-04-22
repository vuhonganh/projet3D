#include "Ray.h"

Ray::Ray(Vec3f position, Vec3f direction, BSHNode * bshRoot, int depth)
{
    this->position = position;
    this->direction = direction;
    this->depth = depth;
    this->bshRoot = bshRoot;
}

//get nearest triangle except exceptionTriangle
pair <int, int> Ray::getNearestTriangle_KDTree(const vector <tinyobj::shape_t> &shapes,
                                               pair <int, int> exceptionTriangle)
{
    pair <int, int> resultIndex = make_pair(-1, -1);
    float resultDistance = -1;
    
    this->getNearestTriangleByBSHNode(shapes, bshRoot, exceptionTriangle, resultIndex, resultDistance);
    
    return resultIndex;
}

void Ray::getNearestTriangleByBSHNode(const vector <tinyobj::shape_t> &shapes,
                                      BSHNode * node,
                                      pair <int, int> exceptionTriangle,
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
            this->getNearestTriangleByBSHNode(shapes, node->leftChild, exceptionTriangle, resultIndex, resultDistance);
        
        if (node->rightChild != NULL)
            this->getNearestTriangleByBSHNode(shapes, node->rightChild, exceptionTriangle, resultIndex, resultDistance);
    }
}

////brute-force
pair <int, int> Ray::getNearestTriangle_BruteForce(const vector <tinyobj::shape_t> &shapes,
                                        pair <int, int> exceptionTriangle)
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
                         pair <int, int> exceptionTriangle,
                         Vec3f * triangle)
{
    pair <int, int> shapeId = this->getNearestTriangle_KDTree(shapes, exceptionTriangle);
    if (shapeId.first == -1) return make_pair(-1, -1);
    
    getTrianglePositionFromShape(shapes, shapeId.first, shapeId.second, triangle);
    return shapeId;
}

bool Ray::canReach(Vec3f point, const vector <tinyobj::shape_t> &shapes, pair <int, int> exceptionTriangle)
{
    Vec3f triangle[3];
    pair <int, int> shapeId = this->getIntersectTriangle(shapes, exceptionTriangle, triangle);
    
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
    //get triangle that intersect the ray
    Vec3f triangle[3];
    pair <int, int> shapeId = this->getIntersectTriangle(shapes, make_pair(-1, -1), triangle);
    if (shapeId.first == -1) return Vec3f(0.0, 0.0, 0.0);
    
    //check if position and lightsource are in different sides of the triangle    
    Vec3f intersection;
    Vec3f color_direct;
    
    if (lineCutTrianglePlane(triangle, this->position, lightSource))
        color_direct = Vec3f(0.0, 0.0, 0.0);
    else
    {    
        //calculate intersection
        this->intersect_remake(triangle, intersection);
        
        //check reflected ray
        Ray reflectedRay(intersection, lightSource - intersection, bshRoot, 0);
        if (reflectedRay.canReach(lightSource, shapes, shapeId) == false)
            color_direct = Vec3f(0.0, 0.0, 0.0);
        else
        {
            //calculate color_direct
            float radian_direct = ggx(this->position, lightSource, intersection, triangle, 1.0, 0.5, 0.5, 1.0);
            unsigned int iMaterial = shapes[shapeId.first].mesh.material_ids[shapeId.second];
            color_direct = Vec3f(materials[iMaterial].diffuse[0] * radian_direct, 
                                       materials[iMaterial].diffuse[1] * radian_direct,
                                       materials[iMaterial].diffuse[2] * radian_direct);
        }
    }
    
    Vec3f color_indirect(0.0, 0.0, 0.0);
    if (depth <= MAX_DEPTH)
    {
        vector <Ray> rays = this->getRandomRaysOut(intersection, triangle, depth + 1, NUMBER_OF_RAYS);
//        cout << "rays.size() = " << rays.size() << endl;
//        vector <Ray> rays = this->getRandomRaysOut_Test(intersection, triangle, depth - 1, NUMBER_OF_RAYS);
        
        int counter = 0;
        for (Ray ray: rays)
        {
            Vec3f color = ray.getColor(shapes, materials, lightSource);
            if (color[0] < EPS && color[1] < EPS && color[2] < EPS) continue;
            
            Vec3f w = ray.direction;
            Vec3f wo = this->position - intersection;
            w.normalize();
            wo.normalize();
            Vec3f n = getNormal(triangle);
            
            float brdf = brdf_GGX(w, wo, n, 0.5, 0.5);
            
            counter++;
            color_indirect += (color * brdf);
        }
        
        if (counter > 0) 
            color_indirect /= counter;
    }
    
    return color_direct + color_indirect;
    
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
    
//    float b0 = dot(s, q);
//    float b1 = dot(r, w);
//    float b2 = 1 - b0 - b1;
    
//    if (b0 < 0 || b1 < 0 || b2 < 0) //IS IT STILL CORRECT WHILE NOT CHECKING THIS?
//        return false;
    
    float t = dot(e1, r);
    if (t >= 0)
    {
//        result = b0 * p[0] + b1 * p[1] + b2 * p[2];
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

//o is the point initial, w is the direction
bool Ray::intersect2(Vec3f * triangle, Vec3f &o, Vec3f &w, Vec3f &result)
{
    Vec3f A = triangle[0];
    Vec3f BA = triangle[1] - A;
    Vec3f CA = triangle[2] - A;

    //vector normal of the plane ABC:
    Vec3f n = cross(BA, CA);
    n /= n.length();

    //the point lies in the plane is: (P-A).n = 0
    //with X = o + w.t, where t is the distance along direction w
    float t = (dot(A,n) - dot(o,n))/(dot(w,n));
    Vec3f X = o + w*t;
    if (t > EPS)
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

vector<Ray> Ray::getRandomRaysOut(Vec3f intersection, Vec3f * triangle, int depth, int NbRays)
{
    Vec3f n = getNormal(triangle);

    //find a unit vector in the plane
    Vec3f ex(triangle[0] - intersection);
    ex /= ex.length();

    //another unit vector in the plane to forme a local coordiante
    Vec3f ey = cross(n, ex);

    vector<Ray> rayOuts;

    for(int i = 0; i < NbRays; i++)
    {
        float xComponent     = getRandomFloat(-1.0, 1.0);
        float yComponent     = getRandomFloat(-1.0, 1.0);
        float normalComponent = getRandomFloat(0.0, 1.0);

        Vec3f dirOut(xComponent * ex + yComponent * ey + normalComponent * n);
        dirOut /= dirOut.length();

        rayOuts.push_back(Ray(intersection, dirOut, this->bshRoot, depth));
    }

    return rayOuts;
}

vector<Ray> Ray::getRandomRaysOut_Test(Vec3f intersection, Vec3f * triangle, int depth, int NbRays)
{
    Vec3f normal = getNormal(triangle);
    Vec3f wi = this->position - intersection;
    wi.normalize();
    
    float cos_theta = dot(normal, wi);
    
    Vec3f y = cross(wi, normal);
    Vec3f x = cross(y, normal);
    
    Vec3f reflect = fabs(sin(acos(cos_theta))) * x + cos_theta * normal;
    reflect.normalize();
    
    vector <Ray> result;
    result.push_back(Ray(intersection, reflect, bshRoot, depth));
    
    return result;
}
