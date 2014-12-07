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
#include "glm/gtx/string_cast.hpp"

#define UNDEFINED -1
#define NUM_PLANETS 100
#define WINDOW_DIM 800
#define SKY_BOUNDS 2000

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
glm::vec3 lightPosCam;

// GLSL program
GLuint pid;

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
GLint h_lightPosCam;

// OpenGL handle to texture data
GLuint outershellTexture;
GLuint flowerTexture;
GLuint uniformKsTexture;
GLuint blackTexture;

unsigned int numRockTextures;
GLuint* rockTextures;
unsigned int numDrawableTextures;
GLuint* planetTextures;
unsigned int numStarTextures;
GLuint* starTextures;
unsigned int numAtmosTextures;
GLuint* atmosTextures;

// Texture matrix
glm::mat3 T(1.0);

Drawable *planetDrawable;
Drawable *catDrawable;
Drawable *rock1Drawable;
Drawable *rock2Drawable;

Drawable* thePlane;
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

string getDrawablePath()
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

void loadScene()
{
	planet.load(getObjPath("sphere.obj"));
	plane.load(getObjPath("sphere.obj"));
	cat.load(getObjPath("cat.obj"));
   rock1.load(getObjPath("rock1.obj"));
   rock2.load(getObjPath("rock2.obj"));
	lightPosCam = glm::vec3(1.0f, 1.0f, 1.0f);
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
	pid = glCreateProgram();
	glAttachShader(pid, VS);
	glAttachShader(pid, FS);
	glLinkProgram(pid);
	GLSL::printError();
	glGetProgramiv(pid, GL_LINK_STATUS, &rc);
	GLSL::printProgramInfoLog(pid);
	if(!rc) {
		printf("Error linking shaders %s and %s\n", vShaderName.c_str(), fShaderName.c_str());
	}
	
	h_vertPosition = GLSL::getAttribLocation(pid, "vertPosition");
	h_vertNormal = GLSL::getAttribLocation(pid, "vertNormal");
	h_vertTexCoords = GLSL::getAttribLocation(pid, "vertTexCoords");
	h_P = GLSL::getUniformLocation(pid, "P");
	h_MV = GLSL::getUniformLocation(pid, "MV");
	h_T = GLSL::getUniformLocation(pid, "T");
	h_texture0 = GLSL::getUniformLocation(pid, "texture0");
	h_texture1 = GLSL::getUniformLocation(pid, "texture1");
	h_texture2 = GLSL::getUniformLocation(pid, "texture2");
	h_lightPosCam = GLSL::getUniformLocation(pid, "lightPosCam");
	
	// Intialize textures
   loadTextures(&rockTextures, &numRockTextures, getRockPath());
   loadTextures(&planetTextures, &numDrawableTextures, getDrawablePath());
   loadTextures(&starTextures, &numStarTextures, getStarPath());
   loadTextures(&atmosTextures, &numAtmosTextures, getAtmosPath());

	loadTexture(&outershellTexture, getImgPath("outershell.bmp").c_str());
	loadTexture(&flowerTexture, getImgPath("flower.bmp").c_str());
   loadTexture(&uniformKsTexture, getImgPath("uniformKs.bmp").c_str());
   loadTexture(&blackTexture, getImgPath("black.bmp").c_str());
	// Check GLSL
	GLSL::checkVersion();

	simulator = new Simulator();

	srand(time(NULL));
	//Create planets
	for (int i = 0; i < NUM_PLANETS; i++) {
		simulator->addMass(new Mass(glm::vec3(rand() % SKY_BOUNDS - SKY_BOUNDS / 2, 
		 	                                   rand() % SKY_BOUNDS - SKY_BOUNDS / 2, 
		 	               	                 rand() % SKY_BOUNDS - SKY_BOUNDS / 2), 1 + (rand() % 100) / 10.0f));
	}

	// simulator->addMass(new Mass(glm::vec3(11.0f, 0.0f, 11.0f), 10));

	thePlane = new Drawable(&plane, &outershellTexture, &outershellTexture, &outershellTexture, 0.0f);
	thePlane->rotate(PI / 2, glm::vec3(0.0f, 1.0f, 0.0f));
	thePlane->translate(glm::vec3(0.0f, -0.5f, 0.0f));
	thePlane->scale(glm::vec3(SKY_BOUNDS * 2, SKY_BOUNDS * 2, SKY_BOUNDS * 2));

	planetDrawable = new Drawable(&planet, &planetTextures[2], &blackTexture, &blackTexture, 0.00005f);
	//catDrawable = new Drawable(&cat, &flowerTexture, &flowerTexture, &earthCloudsTexture, 0.00005f);
   //rock1Drawable = new Drawable(&rock1, &rock1KdTexture, &rock1KdTexture, &rock1KdTexture, 0.0f);
   //rock2Drawable = new Drawable(&rock2, &rock2KdTexture, &rock2KdTexture, &rock2KdTexture, 0.0f);

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

	// Bind the program
	glUseProgram(pid);
	glUniformMatrix4fv(h_P, 1, GL_FALSE, glm::value_ptr(P.topMatrix()));
	glUniformMatrix4fv(h_MV, 1, GL_FALSE, glm::value_ptr(MV.topMatrix()));
	glUniform3fv(h_lightPosCam, 1, glm::value_ptr(lightPosCam));
	
	Drawable::setup(&MV, &h_MV, &h_texture0, &h_texture1, &h_texture2, 
     &h_vertPosition, &h_vertNormal, &h_vertTexCoords, &h_T);

	thePlane->draw();

	simulator->pause();
	camera.calcNormal();
	std::vector<Mass*> *masses = simulator->getMasses();
	BoxNode *boxes = simulator->getOctree();
	for (std::vector<Mass*>::iterator it = masses->begin(); it != masses->end(); ++it) {
		if (camera.inView((*it)->getPosition(), (*it)->getRadius())) {
			planetDrawable->draw((*it)->getPosition(), (*it)->getRadius());
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
	camera.passiveMouseMoved(x, y);
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
		case 'x':
			lightPosCam.x += 0.1;
			break;
		case 'X':
			lightPosCam.x -= 0.1;
			break;
		case 'y':
			lightPosCam.y += 0.1;
			break;
		case 'Y':
			lightPosCam.y -= 0.1;
			break;
		case 'n':
         showNumShapes = !showNumShapes;
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
