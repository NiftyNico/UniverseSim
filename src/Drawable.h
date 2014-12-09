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

#define NO_LIGHT -1

enum class DrawableType 
{ 
    ROCK, 
    PLANET, 
    STAR,
    BLACK_HOLE
};

class Drawable {

private:
    glm::mat4 transpose;

    GLuint* coloring;
    GLuint* reflectivity;
    GLuint* atmosphere;
    glm::mat3 atmosPos;
    float atmosPosModifier;
    DrawableType type;
    Shape* shape;
    int lightIndex;

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
    Drawable(DrawableType type, Shape* shape, GLuint* coloring, GLuint* reflectivity, GLuint* atmosphere, float atmosPosModifier);

    static void setup(MatrixStack* stack, GLint* stackBind, GLint* colorBind, GLint* reflectBind, GLint* atmosBind, 
     GLint* vPosBind, GLint* vNormBind, GLint* vTexCoordsBind, GLint* texTransBind);

    static void setTime(float time);

    void translate(glm::vec3 translateBy);
    void scale(glm::vec3 scaleBy);
    void rotate(float angle, glm::vec3 rotateVec);

    DrawableType getType();

    int getLightIndex();

    int setLightIndex(int newIndex);

    void removeLightIndex();

   	void draw();

    void draw(glm::vec3 pos, float r);
};

#endif
