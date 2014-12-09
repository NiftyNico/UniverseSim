#ifdef __APPLE__
#include <GLUT/glut.h>
#endif
#ifdef __unix__
#include <GL/glut.h>
#endif
#include <iostream>
#include <vector>
#include "GLSL.h"
#include "BoxList.h"
#include "Camera.h"
#include "Mass.h"
#include "MatrixStack.h"
#include "Image.h"
#include "Shape.h"
#include "Simulator.h"
#include <stdio.h>
#include <sstream>
#include "Drawable.h"
#include "Texture.h"
#include "DrawableRandomizer.h"
#include "glm/gtx/string_cast.hpp"

#define UNDEFINED -1
#define NUM_DRAWABLES 100
#define WINDOW_DIM 800
#define SKY_BOUNDS 200

#define MIN_PLANET_MASS 1000.0f
#define MIN_SUN_MASS 3000.0f
#define MIN_BLACK_HOLE_MASS 10000.0f

#define LIGHT_DISTANCE_MODIFIER 10.0f
#define MAX_LIGHTS 100

#define MAX_INIT_VELOCITY 2

#define ROCK_POOL_SIZE 100
#define PLANET_POOL_SIZE 100
#define STAR_POOL_SIZE 100

#define DISTANCE_FROM_DRAWABLE_MOD 5

#define SHADER_PATH "src/shader/"
#define OBJ_PATH "res/obj/"

#define IMG_PATH "res/img/"

using namespace std;

int time0;
Shape planet;
Shape cat;
Shape rock1, rock2;
Shape plane;
Camera camera(WINDOW_DIM);
bool cull = false;
bool line = false;
bool tree = false;
bool showNumShapes = false;

bool showAll = false;
glm::vec3 lightPositions[MAX_LIGHTS];\
bool isOView = true;
glm::vec3 oView = glm::vec3(0.0f, 0.0f, SKY_BOUNDS / 1.1f);
// GLSL program
GLuint regPid;

// GLSL handles to various parameters in the shaders
GLint h_vertPosition;
GLint h_vertNormal;
GLint h_vertTexCoords;
GLint h_P;
GLint h_MV;
GLint h_T;
GLint h_texture0;
GLint h_texture1;
GLint h_texture2;
GLint h_lightPositions;
GLint h_numLights;
GLint h_lightDistanceModifier;
GLint h_isSun;

// OpenGL handle to texture data
GLuint outershellTexture;
GLuint flowerTexture;
GLuint uniformKsTexture;
GLuint blackTexture;

// Texture matrix
glm::mat3 T(1.0);

DrawableRandomizer *dRandomizer;

Drawable *rockPool;
Drawable *planetPool;
Drawable *starPool;

Drawable *planetDrawable;
Drawable *catDrawable;
Drawable *rock1Drawable;
Drawable *rock2Drawable;

Drawable *thePlane;
Drawable *blackHole;
Simulator *simulator;

string getShaderPath(string filename)
{
   return SHADER_PATH + filename;
}

string getObjPath(string filename)
{
   return OBJ_PATH + filename;
}

string getImgPath(string filename)
{
   return IMG_PATH + filename;
}

string getRockPath()
{
   return getImgPath("rocks/");
}

string getPlanetPath()
{
   return getImgPath("planets/");
}

string getStarPath()
{
   return getImgPath("stars/");
}

string getAtmosPath()
{
   return getImgPath("atmospheres/");
}

Drawable* getRock()
{
   return rockPool + rand() % ROCK_POOL_SIZE;
}

Drawable* getPlanet()
{
   return planetPool + rand() % PLANET_POOL_SIZE;
}

Drawable* getStar()
{
   return starPool + rand() % STAR_POOL_SIZE;
}

void loadScene()
{
   planet.load(getObjPath("sphere.obj"));
   plane.load(getObjPath("sphere.obj"));
   cat.load(getObjPath("cat.obj"));
   rock1.load(getObjPath("rock1.obj"));
   rock2.load(getObjPath("rock2.obj"));
}

Drawable* getDrawableFromMass(Mass* mass)
{
   Drawable* ret = getRock();
   if(mass->getMass() >= MIN_PLANET_MASS) {
      ret = getPlanet();
   } if(mass->getMass() >= MIN_SUN_MASS) {
      ret = getStar();
   } if (mass->getMass() >= MIN_BLACK_HOLE_MASS) {
      ret = blackHole;
   }

   return ret;  
}

void updateMassDrawable(Mass* mass)
{
   Drawable *mDrawable = mass->getDrawable(),
            *nDrawable = getDrawableFromMass(mass);

   if(!mDrawable) {
      mass->setDrawable(nDrawable);
      return;
   }
   if(mDrawable->getType() == nDrawable->getType())
      return;

   mass->setDrawable(nDrawable);
}

void initGL()
{
   //////////////////////////////////////////////////////
   // Initialize GL for the whole scene
   //////////////////////////////////////////////////////
   
   // Set background color
   glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
   // Enable z-buffer test
   glEnable(GL_DEPTH_TEST);
   
   //////////////////////////////////////////////////////
   // Initialize the vertex buffers
   //////////////////////////////////////////////////////
   
   planet.init();
   plane.init();
   cat.init();
   rock1.init();
   rock2.init();

   //////////////////////////////////////////////////////
   // Intialize the shaders
   //////////////////////////////////////////////////////
   
   // Create shader handles
   string vShaderName = getShaderPath("vert_shader.glsl");
   string fShaderName = getShaderPath("frag_shader.glsl");
   GLint rc;
   GLuint VS = glCreateShader(GL_VERTEX_SHADER);
   GLuint FS = glCreateShader(GL_FRAGMENT_SHADER);
   
   // Read shader sources
   const char *vshader = GLSL::textFileRead(vShaderName.c_str());
   const char *fshader = GLSL::textFileRead(fShaderName.c_str());
   glShaderSource(VS, 1, &vshader, NULL);
   glShaderSource(FS, 1, &fshader, NULL);
   
   // Compile vertex shader
   glCompileShader(VS);
   GLSL::printError();
   glGetShaderiv(VS, GL_COMPILE_STATUS, &rc);
   GLSL::printShaderInfoLog(VS);
   if(!rc) {
      printf("Error compiling vertex shader %s\n", vShaderName.c_str());
   }
   
   // Compile fragment shader
   glCompileShader(FS);
   GLSL::printError();
   glGetShaderiv(FS, GL_COMPILE_STATUS, &rc);
   GLSL::printShaderInfoLog(FS);
   if(!rc) {
      printf("Error compiling fragment shader %s\n", fShaderName.c_str());
   }
   
   // Create the program and link
   regPid = glCreateProgram();
   glAttachShader(regPid, VS);
   glAttachShader(regPid, FS);
   glLinkProgram(regPid);
   GLSL::printError();
   glGetProgramiv(regPid, GL_LINK_STATUS, &rc);
   GLSL::printProgramInfoLog(regPid);
   if(!rc) {
      printf("Error linking shaders %s and %s\n", vShaderName.c_str(), fShaderName.c_str());
   }
   
   h_vertPosition = GLSL::getAttribLocation(regPid, "vertPosition");
   h_vertNormal = GLSL::getAttribLocation(regPid, "vertNormal");
   h_vertTexCoords = GLSL::getAttribLocation(regPid, "vertTexCoords");
   h_P = GLSL::getUniformLocation(regPid, "P");
   h_MV = GLSL::getUniformLocation(regPid, "MV");
   h_T = GLSL::getUniformLocation(regPid, "T");
   h_texture0 = GLSL::getUniformLocation(regPid, "texture0");
   h_texture1 = GLSL::getUniformLocation(regPid, "texture1");
   h_texture2 = GLSL::getUniformLocation(regPid, "texture2");
   h_lightPositions = GLSL::getUniformLocation(regPid, "lightPositions");
   h_numLights = GLSL::getUniformLocation(regPid, "numLights");
   h_lightDistanceModifier = GLSL::getUniformLocation(regPid, "lightDistanceModifier");
   h_isSun = GLSL::getUniformLocation(regPid, "isSun");

   loadTexture(&outershellTexture, getImgPath("outershell.bmp").c_str());
   loadTexture(&flowerTexture, getImgPath("flower.bmp").c_str());
   loadTexture(&uniformKsTexture, getImgPath("uniformKs.bmp").c_str());
   loadTexture(&blackTexture, getImgPath("black.bmp").c_str());
   
   DrawableRandomizer::initSharedRefs(&rock1, 1,
                                      &planet, 1, 
                                      &planet, 1, 
                                      uniformKsTexture, blackTexture);
   dRandomizer = new DrawableRandomizer(getRockPath(), getPlanetPath(), getStarPath(), getAtmosPath());
   
   rockPool = (Drawable*) malloc(ROCK_POOL_SIZE * sizeof(Drawable));
   for(int i = 0; i < ROCK_POOL_SIZE; i++)
      rockPool[i] = dRandomizer->randomDrawable(DrawableType::ROCK);

   planetPool = (Drawable*) malloc(PLANET_POOL_SIZE * sizeof(Drawable));
   for(int i = 0; i < PLANET_POOL_SIZE; i++)
      planetPool[i] = dRandomizer->randomDrawable(DrawableType::PLANET);

   starPool = (Drawable*) malloc(STAR_POOL_SIZE * sizeof(Drawable));
   for(int i = 0; i < STAR_POOL_SIZE; i++)
      starPool[i] = dRandomizer->randomDrawable(DrawableType::STAR);

   blackHole = new Drawable(DrawableType::BLACK_HOLE, &planet, &outershellTexture, &blackTexture, &blackTexture, 0.0f);

   // Check GLSL
   GLSL::checkVersion();

   simulator = new Simulator();

   //Create planets
   srand(time(NULL));
   for (int i = 0; i < NUM_DRAWABLES; i++) {
      Mass* mass = new Mass(glm::vec3(rand() % SKY_BOUNDS - SKY_BOUNDS / 2, 
                                      rand() % SKY_BOUNDS - SKY_BOUNDS / 2, 
                                      rand() % SKY_BOUNDS - SKY_BOUNDS / 2), 
                            /*glm::vec3(rand() % MAX_INIT_VELOCITY, 
                                      rand() % MAX_INIT_VELOCITY, 
                                      rand() % MAX_INIT_VELOCITY),*/
                            1 + (rand() % 100) / 10.0f);
      
      updateMassDrawable(mass);

      simulator->addMass(mass);
      Simulator::setSelectedMass(mass);
   }

   thePlane = new Drawable(DrawableType::ROCK, &plane, &outershellTexture, &outershellTexture, &outershellTexture, 0.0f);
   thePlane->rotate(PI / 2, glm::vec3(0.0f, 1.0f, 0.0f));
   thePlane->translate(glm::vec3(0.0f, -0.5f, 0.0f));
   thePlane->scale(glm::vec3(SKY_BOUNDS * 2, SKY_BOUNDS * 2, SKY_BOUNDS * 2));

   simulator->start();
}

void reshapeGL(int w, int h)
{
   // Set view size
   glViewport(0, 0, (GLsizei)w, (GLsizei)h);
   // Set camera aspect ratio
   camera.setAspect((float)w/h);
}

void drawText(float x, float y, glm::vec3 color, void* font, char *str)
{
   glColor3f(color.r, color.g, color.b);
   glRasterPos2f(x, y);
   int len, i;
   len = (int)strlen(str);
   for (i = 0; i < len; i++) {
      glutBitmapCharacter(font, str[i]);
   }
}

void drawGL()
{
   // Clear buffers
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   // Enable backface culling
   if(cull) {
      glEnable(GL_CULL_FACE);
   } else {
      glDisable(GL_CULL_FACE);
   }
   if(line) {
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   } else {
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   }

   if (showNumShapes) {
      std::stringstream ss;
      ss << simulator->getMasses()->size();
      std::string numShapes;
      ss >> numShapes;

      drawText(0.45f, -0.9f, glm::vec3(0.5f, 1.0f, 0.5f), GLUT_BITMAP_HELVETICA_18, 
         (char*)(("Number of shapes: " + numShapes).c_str()));
   }

   // Create matrix stacks
   MatrixStack P, MV, T;
   // Apply camera transforms
   P.pushMatrix();
   camera.applyProjectionMatrix(&P);
   MV.pushMatrix();
   camera.applyViewMatrix(&MV);

   Mass* cameraMass = simulator->getSelectedMass();
   if(!isOView) {
      glm::vec3 tempPos = cameraMass->getPosition();
      tempPos.z += DISTANCE_FROM_DRAWABLE_MOD * cameraMass->getRadius();
      camera.setPosition(tempPos);      
   } else {
      camera.setPosition(oView);
   }


   // Bind the program
   glUseProgram(regPid);
   glUniformMatrix4fv(h_P, 1, GL_FALSE, glm::value_ptr(P.topMatrix()));
   glUniformMatrix4fv(h_MV, 1, GL_FALSE, glm::value_ptr(MV.topMatrix()));
   glUniform3fv(h_lightPositions, 3 * MAX_LIGHTS * sizeof(GLfloat), (const GLfloat*) lightPositions);
   glUniform1f(h_lightDistanceModifier, LIGHT_DISTANCE_MODIFIER);
   glUniform1i(h_isSun, 0);   
   Drawable::setup(&MV, &h_MV, &h_texture0, &h_texture1, &h_texture2, 
     &h_vertPosition, &h_vertNormal, &h_vertTexCoords, &h_T);

   thePlane->draw();

   simulator->pause();
   camera.calcNormal();
   std::vector<Mass*> *masses = simulator->getMasses();
   BoxNode *boxes = simulator->getOctree();

   Drawable* toDraw;
   GLint numLights = 0;
   for (std::vector<Mass*>::iterator it = masses->begin(); it != masses->end(); ++it) {
      //if (camera.inView((*it)->getPosition(), (*it)->getRadius())) {
         updateMassDrawable((*it));

         toDraw = (*it)->getDrawable();
         if(toDraw->getType() == DrawableType::STAR && numLights < MAX_LIGHTS){
            lightPositions[numLights++] = (*it)->getPosition();            
         }
      //}
   }
   glUniform1i(h_numLights, numLights);

   for (std::vector<Mass*>::iterator it = masses->begin(); it != masses->end(); ++it) {
      if (camera.inView((*it)->getPosition(), (*it)->getRadius())) {
         toDraw = (*it)->getDrawable();

         int shouldHighlight = toDraw->getType() == DrawableType::STAR || showAll ? 1 : 0;
         glUniform1i(h_isSun, shouldHighlight);

         toDraw->draw((*it)->getPosition(), (*it)->getRadius());
      }
   }

   // Unbind the program
   glUseProgram(0);
   
   if (tree) {
      BoxNode *tmp = boxes;

      glColor3f(1.0f, 1.0f, 1.0f);
      while (tmp) {
         glm::vec4 t = P.topMatrix() * MV.topMatrix() * glm::vec4(tmp->low, 1.0f);
         glm::vec3 l = glm::vec3(t) / t.w;

         t = P.topMatrix() * MV.topMatrix() * glm::vec4(tmp->high, 1.0f);
         glm::vec3 h = glm::vec3(t) / t.w;

         glBegin(GL_LINE_LOOP);
            glVertex3f(l.x, l.y, l.z);
            glVertex3f(h.x, l.y, l.z);
            glVertex3f(h.x, h.y, l.z);
            glVertex3f(l.x, h.y, l.z);
         glEnd();

         glBegin(GL_LINE_LOOP);
            glVertex3f(l.x, l.y, h.z);
            glVertex3f(h.x, l.y, h.z);
            glVertex3f(h.x, h.y, h.z);
            glVertex3f(l.x, h.y, h.z);
         glEnd();

         glBegin(GL_LINE);
            glVertex3f(l.x, l.y, l.z);
            glVertex3f(l.x, l.y, h.z);

            glVertex3f(h.x, l.y, l.z);
            glVertex3f(h.x, l.y, h.z);

            glVertex3f(h.x, h.y, l.z);
            glVertex3f(h.x, h.y, h.z);

            glVertex3f(l.x, h.y, l.z);
            glVertex3f(l.x, h.y, h.z);
         glEnd();

         tmp = tmp->next;
      }
   }

   freeList(boxes);
   simulator->resume();

   // Pop stacks
   MV.popMatrix();
   P.popMatrix();

   // Double buffer
   glutSwapBuffers();
}

void mouseGL(int button, int state, int x, int y)
{
   int modifier = glutGetModifiers();
   bool shift = modifier & GLUT_ACTIVE_SHIFT;
   bool ctrl  = modifier & GLUT_ACTIVE_CTRL;
   bool alt   = modifier & GLUT_ACTIVE_ALT;
   camera.mouseClicked(x, y, shift, ctrl, alt);
}

void motionGL(int x, int y)
{
   camera.mouseMoved(x, y);

   // Refresh screen
   glutPostRedisplay();
}

void glutPassiveMotionGL(int x, int y){
   //camera.passiveMouseMoved(x, y);
}

void keyboardGL(unsigned char key, int x, int y)
{    
   switch(key) {
      case 27:
         // ESCAPE
         exit(0);
         break;
      case 'c':
         cull = !cull;
         break;
      case 'l':
         line = !line;
         break;
      case 't':
         tree = !tree;
         break;
      case 'n':
         showNumShapes = !showNumShapes;
         break;
      case 'm':
         Simulator::nextMass(simulator);
         break;
      case 'b':
         showAll = !showAll;
         break;
      case 'o':
         isOView = !isOView;
         break;
      default:
         camera.movement(key);
   }
   // Refresh screen
   glutPostRedisplay();
}

void idleGL()
{
   int time1 = glutGet(GLUT_ELAPSED_TIME);
   int dt = time1 - time0;
   // Update every 60Hz
   if(dt > 1000.0/60.0) {
      time0 = time1;
      Drawable::setTime(time1);
      glutPostRedisplay();
   }
}

int main(int argc, char **argv)
{
   glutInit(&argc, argv);
   glutInitWindowSize(WINDOW_DIM, WINDOW_DIM);
   glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
   glutCreateWindow("Donovan McKelvey & Nicolas Higuera");
   glutMouseFunc(mouseGL);
   glutMotionFunc(motionGL);
   glutPassiveMotionFunc(glutPassiveMotionGL);
   glutKeyboardFunc(keyboardGL);
   glutReshapeFunc(reshapeGL);
   glutDisplayFunc(drawGL);
   glutIdleFunc(idleGL);
   loadScene();
   initGL();
   glutMainLoop();
   return 0;
}
