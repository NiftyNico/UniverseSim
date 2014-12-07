#ifndef DRAWABLE_H
#define DRAWABLE_H

#ifdef __APPLE__
#include <GLUT/glut.h>
#endif
#ifdef __unix__
#include <GL/glut.h>
#endif
#include "GLSL.h"
#include "tiny_obj_loader.h"
#include "MatrixStack.h"
#include "Camera.h"
#include "Shape.h"

class Drawable {

private:
    glm::mat4 transpose;

    GLuint* coloring;
    GLuint* reflectivity;
    GLuint* atmosphere;
    glm::mat3 atmosPos;
    float atmosPosModifier;

    Shape* shape;
    static MatrixStack* stack;
    static GLint* stackBind;
    static GLint* colorBind;
    static GLint* reflectBind;
    static GLint* atmosBind;
    static GLint* vPosBind;
    static GLint* vNormBind;
    static GLint* vTexCoordsBind;
    static GLint* texTransBind;

    static float time;

public:
    Drawable(Shape* shape, GLuint* coloring, GLuint* reflectivity, GLuint* atmosphere, float atmosPosModifier);

    static void setup(MatrixStack* stack, GLint* stackBind, GLint* colorBind, GLint* reflectBind, GLint* atmosBind, 
     GLint* vPosBind, GLint* vNormBind, GLint* vTexCoordsBind, GLint* texTransBind);

    static void setTime(float time);

    void translate(glm::vec3 translateBy);
    void scale(glm::vec3 scaleBy);
    void rotate(float angle, glm::vec3 rotateVec);

   	void draw();

    void draw(glm::vec3 pos, float r);
};

#endif
