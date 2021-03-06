// ----------------------------------------------
// Informatique Graphique 3D & Realite Virtuelle.
// Proje
// Lancer de Rayon de Monte Carlo
// Copyright (C) 2015 Tamy Boubekeur
// All rights reserved.
// ----------------------------------------------

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <cstdlib>
#include <cmath>

#ifdef __APPLE__
#include <GLUT/GLUT.h>
#include <OpenGL/OpenGL.h>
#elif __linux
#include <GL/glut.h>
#elif _WIN32
#include <GL/glut.h>
#endif

#include "Vec3.h"
#include "tiny_obj_loader.h"
#include "RaySource.h"
#include "BSHNode.h"
#include <ctime>

using namespace std;

// App parameters
static const unsigned int DEFAULT_SCREENWIDTH = 600;
static const unsigned int DEFAULT_SCREENHEIGHT = 600;
static const char * DEFAULT_SCENE_FILENAME = "scenes/cornell_box/cornell_box.obj";
//static const char * DEFAULT_SCENE_FILENAME = "scenes/CornellBox-Original/CornellBox-Original.obj";
//static const char * DEFAULT_SCENE_FILENAME = "scenes/mitsuba/mitsuba.obj";
//static const char * DEFAULT_SCENE_FILENAME = "scenes/rungholt/rungholt.obj";
//"scenes/cube/cube.obj";
//"scenes/mitsuba/mitsuba-sphere.obj";
static string appTitle ("MCRT - Monte Carlo Ray Tracer");
static GLint window;
static unsigned int screenWidth;
static unsigned int screenHeight;
static bool rayDisplayMode = false;

// Camera parameters
static float fovAngle;
static float aspectRatio;
static float nearPlane;
static float farPlane;
static Vec3f camEyePolar; // Expressing the camera position in polar coordinate, in the frame of the target
static Vec3f camTarget;

// Scene elements
static Vec3f lightPos = Vec3f (1.f, 1.f, 1.f);
static Vec3f lightColor = Vec3f (1.f, 1.f, 1.f);
static Vec3f sceneCenter = Vec3f (0.f, 0.f, 0.f);

//cornell
static Vec3f lightSource = Vec3f(300, 518, 250);
//static Vec3f lightSource = Vec3f(500, 250, 200);

static float sceneRadius = 1.f;
static vector<tinyobj::shape_t> shapes;
static vector<tinyobj::material_t> materials;
//static Vec3f fixedEye = Vec3f(113.655, 428.282, -740.034);

// Mouse parameters
static bool mouseLeftButtonClicked = false;
static int clickedX, clickedY;
static float baseCamPhi;
static float baseCamTheta;

// Raytraced image
static unsigned char * rayImage = NULL;
BSHNode * bshRoot;

void printUsage ()
{
    std::cerr << std::endl // send a line break to the standard error output
              << appTitle << std::endl
              << "Author : Tamy Boubekeur" << std::endl << std::endl
              << "Usage : ./myRayTracer [<file.obj>]" << std::endl
              << "Commandes clavier :" << std::endl 
              << "------------------" << std::endl
              << " ?: Print help" << std::endl
              << " <space>: Toggle raytracing/rasterization (GL)  display mode" << std::endl
              << " r: Ray trace an image from the current point of view" << std::endl
              << " s: Save the current ray traced image under raytraced_image.ppm" << std::endl
              << " <drag>+<left button>: rotate model" << std::endl 
              << " <drag>+<right button>: move model" << std::endl
              << " <drag>+<middle button>: zoom" << std::endl
              << " q, <esc>: Quit" << std::endl << std::endl; 
}

void initOpenGL()
{
    glCullFace (GL_BACK);     // Specifies the faces to cull (here the ones pointing away from the camera)
    glEnable (GL_CULL_FACE); // Enables face culling (based on the orientation defined by the CW/CCW enumeration).
    glDepthFunc (GL_LESS); // Specify the depth test for the z-buffer
    glEnable (GL_DEPTH_TEST); // Enable the z-buffer in the rasterization
    glClearColor (0.0f, 0.0f, 0.0f, 1.0f); // Background color
    glEnable (GL_COLOR_MATERIAL);
}

void computeSceneNormals()
{
    for (unsigned int s = 0; s < shapes.size (); s++) 
        if (shapes[s].mesh.normals.empty ())
        {
            shapes[s].mesh.normals.resize (shapes[s].mesh.positions.size (), 0.f);
            for (size_t f = 0; f < shapes[s].mesh.indices.size() / 3; f++)
            {
                Vec3f q[3];
                for (size_t v = 0; v < 3; v++)
                {
                    unsigned int index = 3*shapes[s].mesh.indices[3*f+v];
                    for (unsigned int i = 0; i < 3; i++)
                        q[v][i] = shapes[s].mesh.positions[index+i];
                }
                Vec3f e01 = q[1] - q[0];
                Vec3f e02 = q[2] - q[0];
                Vec3f nf = normalize (cross (e01, e02));
                for (size_t v = 0; v < 3; v++)
                {
                    unsigned int index = 3*shapes[s].mesh.indices[3*f+v];
                    for (unsigned int i = 0; i < 3; i++)
                        shapes[s].mesh.normals[index+i] += nf[i];
                }
            }
            for (unsigned int i = 0; i < shapes[s].mesh.normals.size () / 3; i++)
            {
                Vec3f n;
                for (unsigned int j = 0; j < 3; j++)
                    n[j] = shapes[s].mesh.normals[3*i+j];
                n.normalize ();
                for (unsigned int j = 0; j < 3; j++)
                    shapes[s].mesh.normals[3*i+j] = n[j];
            }
        }
}

void computeSceneBoundingSphere ()
{
    sceneCenter = Vec3f (0.f, 0.f, 0.f);
    unsigned int count = 0;
    for (unsigned int s = 0; s < shapes.size (); s++)
        for (unsigned int p = 0; p < shapes[s].mesh.positions.size () / 3; p++)
        {
            sceneCenter += Vec3f (shapes[s].mesh.positions[3*p],
                                  shapes[s].mesh.positions[3*p+1],
                                  shapes[s].mesh.positions[3*p+2]);
            count++;
        }
    sceneCenter /= count;
    sceneRadius = 0.f;
    for (unsigned int s = 0; s < shapes.size (); s++)
        for (unsigned int p = 0; p < shapes[s].mesh.positions.size () / 3; p++)
        {
            float d = dist (sceneCenter, Vec3f (shapes[s].mesh.positions[3*p],
                                                shapes[s].mesh.positions[3*p+1],
                                                shapes[s].mesh.positions[3*p+2]));
            if (d > sceneRadius)
                sceneRadius = d;
        }
}

// Loads an OBJ file using tinyOBJ (http://syoyo.github.io/tinyobjloader/)
bool loadScene(const string & filename, const string & basepath = "") {
    shapes.clear ();
    materials.clear ();
    std::cout << "Loading " << filename << std::endl;
    std::string err = tinyobj::LoadObj(shapes, materials, filename.c_str (), basepath.c_str ());
    if (!err.empty())
    {
        std::cerr << err << std::endl;
        return false;
    }
    computeSceneNormals ();
    computeSceneBoundingSphere ();
    return true;
}

void initCamera ()
{
    fovAngle = 45.f;
    nearPlane = sceneRadius/10000.0f;
    farPlane = 10*sceneRadius;
    camTarget = sceneCenter;
    camEyePolar = Vec3f (2.f * sceneRadius, M_PI/2.f, M_PI/2.f);
}

void initLighting ()
{
    lightPos = 2.f * Vec3f (sceneRadius, sceneRadius, sceneRadius);
    glEnable (GL_LIGHTING);
    GLfloat position[4] = {lightPos[0], lightPos[1], lightPos[2], 1.0f};
    GLfloat color[4] = {lightColor[0], lightColor[1], lightColor[2], 1.0f};
    glLightfv (GL_LIGHT0, GL_POSITION, position);
    glLightfv (GL_LIGHT0, GL_DIFFUSE, color);
    glLightfv (GL_LIGHT0, GL_SPECULAR, color);
    glEnable (GL_LIGHT0);
}

void init (const string & filename)
{
    initOpenGL ();
    unsigned int i = filename.find_last_of ("/");
    loadScene (filename, filename.substr (0, i+1));
    initCamera ();
    initLighting ();
    
    vector <pair <int, int> > triangleIds;
    for (size_t s = 0; s < shapes.size(); s++)
        for (size_t f = 0; f < shapes[s].mesh.indices.size() / 3; f++)
            triangleIds.push_back(make_pair(s, f));
    
    bshRoot = new BSHNode(shapes, triangleIds);
    
    srand(time(NULL));
}

void setupCamera ()
{
    glMatrixMode (GL_PROJECTION); // Set the projection matrix as current. All upcoming matrix manipulations will affect it.
    glLoadIdentity ();
    gluPerspective (fovAngle, aspectRatio, nearPlane, farPlane); // Set the current projection matrix with the camera intrinsics
    glMatrixMode (GL_MODELVIEW); // Set the modelview matrix as current. All upcoming matrix manipulations will affect it.
    glLoadIdentity ();
    Vec3f eye = polarToCartesian (camEyePolar);
    swap (eye[1], eye[2]); // swap Y and Z to keep the Y vertical
    eye += camTarget;
    
    gluLookAt (eye[0], eye[1], eye[2], 
               camTarget[0], camTarget[1], camTarget[2], 
               0.0, 1.0, 0.0); // Set up the current modelview matrix with camera transform
}

void reshape (int w, int h)
{
    screenWidth = w;
    screenHeight = h;
    aspectRatio = static_cast<float>(w)/static_cast<float>(h);
    glViewport (0, 0, (GLint)w, (GLint)h); // Dimension of the drawing region in the window
    setupCamera ();
    if (rayImage != NULL) 
        delete [] rayImage;
    unsigned int l = 3*screenWidth*screenHeight;
    rayImage = new unsigned char [l];
    memset (rayImage, 0, l);
}

void polar2Cartesian (float phi, float theta, float d, float & x, float & y, float & z) 
{
  x = d*sin (theta) * cos (phi);
  y = d*cos (theta);
  z = d*sin (theta) * sin (phi);
}

void  glSphere(float xc, float yc, float zc, float radius)
{
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glTranslatef(xc, yc, zc);
  
  //sphere 
  glBegin(GL_TRIANGLES);	
  
  for(int theta_step = 0; theta_step < 18; theta_step++)
    for(int phi_step = 0; phi_step < 36; phi_step++)
      {
	float theta      = M_PI*theta_step/18.0;
	float phi        = 2*M_PI*phi_step/36.0;
	float theta_next = M_PI*(theta_step+1)/18.0;
	float phi_next   = 2*M_PI*(phi_step+1)/36.0;
	
	float d = radius;
	
	float x[4];
	float y[4];
	float z[4];
	
	// points:
	polar2Cartesian (phi, theta, d, x[1], y[1], z[1]);
	polar2Cartesian (phi_next, theta, d, x[0], y[0], z[0]);
	polar2Cartesian (phi, theta_next, d, x[2], y[2], z[2]);
	polar2Cartesian (phi_next, theta_next, d, x[3], y[3], z[3]);

	//first triangle 3 2 1
	for (int i = 3; i > 0; i--)
	  {
	    glColor3f(abs(x[i-1])/d, abs(y[i-1])/d, abs(z[i-1])/d);
	    glVertex3f(x[i-1], y[i-1], z[i-1]);       
	  }
	
	//second triangle 4 3 1
	for (int i = 4; i > 0; i--)
	  {
	    if (i == 2)
	      continue;
	    glColor3f(abs(x[i-1])/d, abs(y[i-1])/d, abs(z[i-1])/d);
	    glVertex3f(x[i-1], y[i-1], z[i-1]);
	  }
      }
  glEnd();
  glPopMatrix();
}

void rasterize ()
{
    setupCamera ();   
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Erase the color and z buffers.
    glBegin (GL_TRIANGLES);
    glColor3f (1.f, 1.f, 1.f);
    
    for (size_t s = 0; s < shapes.size(); s++)
        for (size_t f = 0; f < shapes[s].mesh.indices.size() / 3; f++)
        {          
//            if (s != 0) continue;
            
            if (!materials.empty ())
            {
                // MAIN FUNCTION TO CHANGE !
                unsigned int i = shapes[s].mesh.material_ids[f];
                glColor3f (materials[i].diffuse[0], materials[i].diffuse[1], materials[i].diffuse[2]);
            }
            
            for (size_t v = 0; v < 3; v++)
            {
                unsigned int index = 3*shapes[s].mesh.indices[3*f+v];
                glNormal3f (shapes[s].mesh.normals[index],
                            shapes[s].mesh.normals[index+1],
                            shapes[s].mesh.normals[index+2]);
                glVertex3f (shapes[s].mesh.positions[index],
                            shapes[s].mesh.positions[index+1],
                            shapes[s].mesh.positions[index+2]);
                
                Vec3f temp = Vec3f(shapes[s].mesh.positions[index],
                                    shapes[s].mesh.positions[index+1],
                                    shapes[s].mesh.positions[index+2]);
            }
        }
    
    glSphere(lightSource[0], lightSource[1], lightSource[2], 1);
    glSphere(lightSource[0], lightSource[1], lightSource[2], 0.5);
    glEnd ();
    glFlush (); // Ensures any previous OpenGL call has been executed
    glutSwapBuffers ();  // swap the render buffer and the displayed (screen) one
}

void displayRayImage()
{    
    glDisable (GL_DEPTH_TEST);
    glDrawPixels (screenWidth, screenHeight, GL_RGB, GL_UNSIGNED_BYTE, static_cast<void*>(rayImage));
    glutSwapBuffers ();
    glEnable (GL_DEPTH_TEST);
}

// MAIN FUNCTION TO CHANGE !
void rayTrace ()
{
    clock_t begin = clock();
    
    //get camera's position
    Vec3f eye = polarToCartesian (camEyePolar);
    swap (eye[1], eye[2]); // swap Y and Z to keep the Y vertical
    eye += camTarget;
    
//    eye = fixedEye;
    
    //create multiple light sources
//    vector <Vec3f> lightSources;
//    float r = 5;
//    float step = 1;
//    float eps = step / 2;
    
//    for (float x = lightSource[0] - r; x <= lightSource[0] + r + eps; x += step)
//        for (float z = lightSource[2] - r; z <= lightSource[2] + r + eps; z += step)
//            lightSources.push_back(Vec3f(x, lightSource[1], z));

    //create light sources
    vector <Vec3f> lightSources;
    lightSources.push_back(lightSource);
    
    RaySource raySource(lightSources, eye, camTarget, screenWidth, screenHeight);
    
    //export to array
    raySource.exportToRGB(shapes, materials, bshRoot, rayImage);
    
    clock_t end = clock();
    
    //message
    printf("%.3lf\n", double(end - begin) / CLOCKS_PER_SEC);
    cout << "lightSource = " << lightSource << endl;
    cout << 'a';
    cout << 'a';
    cout << 'a';
}

void display ()
{
    if (rayDisplayMode)
        displayRayImage ();
    else
        rasterize ();
}

void saveRayImage (const string & filename)
{
    if (rayImage != NULL)
    {
        std::ofstream out (filename.c_str ());
        out << "P3" << endl
            << screenWidth << " " << screenHeight << endl
            << "255" << endl;
        for (unsigned int i = 0; i < 3*screenWidth*screenHeight; i++)
            out << static_cast<int>(rayImage[i]) << " ";
        out.close ();
    }
}

void keyboard (unsigned char keyPressed, int x, int y)
{
    switch (keyPressed)
    {
    case ' ':
        rayDisplayMode = !rayDisplayMode;
        glutPostRedisplay ();
        break;
    case 'r':
        rayTrace ();
        glutPostRedisplay ();
        break;
    case 's':
        saveRayImage ("raytraced_image.ppm");
        break;
    case 'q':
    case 27:
        exit (0);
        break;
    default:
        printUsage ();
        break;
    }
}

void mouse (int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {	
            mouseLeftButtonClicked = true;
            clickedX = x;
            clickedY = y;
            baseCamPhi = camEyePolar[1];
            baseCamTheta = camEyePolar[2];
            cout << "x = " << x << ", y = " << y << endl;
        } else {
            mouseLeftButtonClicked = false;
        }
    }
}

void motion (int x, int y)
{
    if (mouseLeftButtonClicked == true) {
        camEyePolar[1] =  baseCamPhi  + (float (clickedY-y)/screenHeight) * M_PI;
        camEyePolar[2] = baseCamTheta + (float (x-clickedX)/screenWidth) * M_PI;
        glutPostRedisplay (); // calls the display function
    }
}

// This function is executed in an infinite loop. 
void idle ()
{
}

int main (int argc, char ** argv)
{
    glutInit (&argc, argv); // Initialize a glut app
    glutInitDisplayMode (GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE); // Setup a RGBA framebuffer to display, with a depth buffer (z-buffer), in double buffer mode (fill a buffer then update the screen)
    glutInitWindowSize (DEFAULT_SCREENWIDTH, DEFAULT_SCREENHEIGHT); // Set the window app size on screen
    window = glutCreateWindow (appTitle.c_str ()); // create the window
    if (argc > 1)
        init (argv[1]); // Your initialization code (OpenGL states, geometry, material, lights, etc)
    else
        init (DEFAULT_SCENE_FILENAME);
    glutReshapeFunc (reshape); // Callback function executed whenever glut need to setup the projection matrix
    glutDisplayFunc (display); // Callback function executed when the window app need to be redrawn
    glutKeyboardFunc (keyboard); // Callback function executed when the keyboard is used
    glutMouseFunc (mouse); // Callback function executed when a mouse button is clicked 
    glutMotionFunc (motion); // Callback function executed when the mouse move
    glutIdleFunc (idle); // Callback function executed continuously when no other event happens (good for background procesing or animation for instance).
    printUsage (); // By default, display the usage help of the program   
    glutMainLoop ();
    
    delete bshRoot;
    return 0;
}
