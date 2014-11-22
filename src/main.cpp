#ifdef __APPLE__
#include <GLUT/glut.h>
#endif
#ifdef __unix__
#include <GL/glut.h>
#endif
#include <iostream>
#include <vector>
#include "GLSL.h"
#include "Camera.h"
#include "MatrixStack.h"
#include "Image.h"
#include "Shape.h"
#include <stdio.h>

#include "planet.h"

#define UNDEFINED -1
#define NUM_PLANETS 10000
#define WINDOW_DIM 800
#define SKY_BOUNDS 100

using namespace std;

Shape planet;
Shape cat;
Shape plane;
Camera camera(WINDOW_DIM);
bool cull = false;
bool line = false;
glm::vec3 lightPosCam;

static float moveCloudsBy = 0;

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
GLuint texture0ID;
GLuint texture1ID;
GLuint texture2ID;
GLuint texture3ID;
GLuint texture4ID;
// Texture matrix
glm::mat3 T(1.0);

std::vector<Planet> planets;
Planet* thePlane;

string getShaderPath(string filename)
{
	return "src/shader/" + filename;
}

string getObjPath(string filename)
{
	return "res/obj/" + filename;
}

string getImgPath(string filename)
{
	return "res/img/" + filename;
}

void loadScene()
{
	planet.load(getObjPath("sphere.obj"));
	plane.load(getObjPath("sphere.obj"));
	cat.load(getObjPath("cat.obj"));
	lightPosCam = glm::vec3(1.0f, 1.0f, 1.0f);
}

void loadTexture(GLuint* textureId, const char* fileName)
{
	Image *image1 = (Image *)malloc(sizeof(Image));
	if(image1 == NULL) {
		printf("Error allocating space for image");
	}
	if(!ImageLoad(fileName, image1)) {
		printf("Error loading texture image\n");
	}
	glActiveTexture(GL_TEXTURE1);
	glGenTextures(1, textureId);
	glBindTexture(GL_TEXTURE_2D, *textureId);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, image1->sizeX, image1->sizeY,
				 0, GL_RGB, GL_UNSIGNED_BYTE, image1->data);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	free(image1);	
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
	
	//////////////////////////////////////////////////////
	// Intialize textures
	//////////////////////////////////////////////////////
	
	loadTexture(&texture0ID, getImgPath("earthKd.bmp").c_str());
	loadTexture(&texture1ID, getImgPath("earthKs.bmp").c_str());
	loadTexture(&texture2ID, getImgPath("earthClouds.bmp").c_str());
	loadTexture(&texture3ID, getImgPath("outershell.bmp").c_str());
	loadTexture(&texture4ID, getImgPath("flower.bmp").c_str());

	// Check GLSL
	GLSL::checkVersion();

	//Create planets
	for (int i = 0; i < NUM_PLANETS; i++){
		Planet p(&planet, &texture0ID, &texture1ID, &texture2ID);
		p.rotate(rand(), glm::vec3(1.0f, 0.0f, 0.0f));
		p.rotate(rand(), glm::vec3(0.0f, 1.0f, 0.0f));
		p.rotate(rand(), glm::vec3(0.0f, 0.0f, 1.0f));
		p.translate(glm::vec3(rand() % SKY_BOUNDS - SKY_BOUNDS / 2, 
			                  rand() % SKY_BOUNDS + 2.0f, 
			               	  rand() % SKY_BOUNDS - SKY_BOUNDS / 2));
		planets.push_back(p);

		Planet b(&cat, &texture4ID, &texture4ID, &texture4ID);
		b.rotate(rand(), glm::vec3(1.0f, 0.0f, 0.0f));
		b.rotate(rand(), glm::vec3(0.0f, 1.0f, 0.0f));
		b.rotate(rand(), glm::vec3(0.0f, 0.0f, 1.0f));
		b.translate(glm::vec3(rand() % SKY_BOUNDS - SKY_BOUNDS / 2, 
			                  rand() % SKY_BOUNDS + 2.0f, 
			               	  rand() % SKY_BOUNDS - SKY_BOUNDS / 2));
		planets.push_back(b);
	}

	thePlane = new Planet(&plane, &texture3ID, &texture3ID, &texture3ID);
	thePlane->rotate(PI / 2, glm::vec3(0.0f, 1.0f, 0.0f));
	thePlane->translate(glm::vec3(0.0f, -0.5f, 0.0f));
	thePlane->scale(glm::vec3(SKY_BOUNDS, SKY_BOUNDS, SKY_BOUNDS));
}

void reshapeGL(int w, int h)
{
	// Set view size
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	// Set camera aspect ratio
	camera.setAspect((float)w/h);
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
	
	// Create matrix stacks
	MatrixStack P, MV, T;
	// Apply camera transforms
	P.pushMatrix();
	camera.applyProjectionMatrix(&P);
	MV.pushMatrix();
	camera.applyViewMatrix(&MV);

	glm::mat3 cloudMover(1.0);
	//T.translate(glm::vec3(moveCloudsBy, 0.0f, 0.0f));	
	cloudMover[0][0] = 1.0f;
	cloudMover[0][1] = 0.0f;
	cloudMover[0][2] = 0.0f;
	cloudMover[1][0] = 0.0f;
	cloudMover[1][1] = 1.0f;
	cloudMover[1][2] = 0.0f;
	cloudMover[2][0] = moveCloudsBy;
	cloudMover[2][1] = 0.0f;
	cloudMover[2][2] = 1.0f;

	// Bind the program
	glUseProgram(pid);
	glUniformMatrix4fv(h_P, 1, GL_FALSE, glm::value_ptr(P.topMatrix()));
	glUniformMatrix4fv(h_MV, 1, GL_FALSE, glm::value_ptr(MV.topMatrix()));
	glUniform3fv(h_lightPosCam, 1, glm::value_ptr(lightPosCam));
	

	Planet::setup(&MV, &h_MV, &h_texture0, &h_texture1, &h_texture2, 
     &h_vertPosition, &h_vertNormal, &h_vertTexCoords, &h_T);

	thePlane->draw();

    for(std::vector<Planet>::iterator it = planets.begin(); it != planets.end(); it++)
    	it->draw();

	// Unbind the program
	glUseProgram(0);

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
		default:
			camera.movement(key);
	}
	// Refresh screen
	glutPostRedisplay();
}

void idleGL()
{
	//float t = glutGet(GLUT_ELAPSED_TIME);
	moveCloudsBy += .00005;
	glutPostRedisplay();
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitWindowSize(WINDOW_DIM, WINDOW_DIM);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutCreateWindow("Nicolas Higuera");
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
