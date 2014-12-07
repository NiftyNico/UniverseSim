#include "Drawable.h"

MatrixStack* Drawable::stack;
GLint* Drawable::stackBind;
GLint* Drawable::colorBind;
GLint* Drawable::reflectBind;
GLint* Drawable::atmosBind;
GLint* Drawable::vPosBind;
GLint* Drawable::vNormBind;
GLint* Drawable::vTexCoordsBind;
GLint* Drawable::texTransBind;
float Drawable::time;

Drawable::Drawable(Shape* shape, GLuint* coloring, GLuint* reflectivity, GLuint* atmosphere, float atmosPosModifier) {
    Drawable::shape = shape;
    Drawable::coloring = coloring;
    Drawable::reflectivity = reflectivity;
    Drawable::atmosphere = atmosphere;
    Drawable::atmosPosModifier = atmosPosModifier;
    transpose[0][0] = 1;
    transpose[1][1] = 1;
    transpose[2][2] = 1;
    transpose[3][3] = 1;
    
    atmosPos[0][0] = 1.0f;
    atmosPos[0][1] = 0.0f;
    atmosPos[0][2] = 0.0f;
    atmosPos[1][0] = 0.0f;
    atmosPos[1][1] = 1.0f;
    atmosPos[1][2] = 0.0f;
    atmosPos[2][0] = 0.0f;
    atmosPos[2][1] = 0.0f;
    atmosPos[2][2] = 1.0f;
}

void Drawable::setup(MatrixStack* stack, GLint* stackBind, GLint* colorBind, GLint* reflectBind, GLint* atmosBind, 
 GLint* vPosBind, GLint* vNormBind, GLint* vTexCoordsBind, GLint* texTransBind) {
    Drawable::stack = stack;
    Drawable::stackBind = stackBind;
    Drawable::colorBind = colorBind;
    Drawable::reflectBind = reflectBind;
    Drawable::atmosBind = atmosBind;
    Drawable::vPosBind = vPosBind;
    Drawable::vNormBind = vNormBind;
    Drawable::vTexCoordsBind = vTexCoordsBind;
    Drawable::texTransBind = texTransBind;
}

void Drawable::setTime(float time)
{
    Drawable::time = time;
}

void Drawable::rotate(float angle, glm::vec3 rotateVec) {
    transpose *= glm::rotate(angle, rotateVec);
}

void Drawable::translate(glm::vec3 transVec) {
    transpose *= glm::translate(transVec);
}

void Drawable::scale(glm::vec3 scaleVec) {
    transpose *= glm::scale(scaleVec);
}

void Drawable::draw() {
    stack->pushMatrix();
    stack->multMatrix(transpose);

    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, *coloring);
    glUniform1i(*colorBind, 0);
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, *reflectivity);
    glUniform1i(*reflectBind, 1);
    glActiveTexture(GL_TEXTURE0 + 2);
    glBindTexture(GL_TEXTURE_2D, *atmosphere);
    glUniform1i(*atmosBind, 2);

    atmosPos[2][0] = atmosPosModifier * time;

    // Texture matrix
    // We're sending in GL_TRUE for the transpose argument, because we want
    // the translation to be the right-most column instead of the bottom row.
    glUniformMatrix3fv(*texTransBind, 1, GL_FALSE, glm::value_ptr(atmosPos));

    glUniformMatrix4fv(*stackBind, 1, GL_FALSE, glm::value_ptr(stack->topMatrix()));

    // Draw shape
    shape->draw(*vPosBind, *vNormBind, *vTexCoordsBind);
    
    // Unbind textures
    glActiveTexture(GL_TEXTURE0 + 2);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    stack->popMatrix();
}

void Drawable::draw(glm::vec3 pos, float r) {
    stack->pushMatrix();
    stack->translate(pos);
    stack->scale(glm::vec3(r));

    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, *coloring);
    glUniform1i(*colorBind, 0);
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, *reflectivity);
    glUniform1i(*reflectBind, 1);
    glActiveTexture(GL_TEXTURE0 + 2);
    glBindTexture(GL_TEXTURE_2D, *atmosphere);
    glUniform1i(*atmosBind, 2);

    atmosPos[2][0] = atmosPosModifier * time;

    // Texture matrix
    // We're sending in GL_TRUE for the transpose argument, because we want
    // the translation to be the right-most column instead of the bottom row.
    glUniformMatrix3fv(*texTransBind, 1, GL_FALSE, glm::value_ptr(atmosPos));

    glUniformMatrix4fv(*stackBind, 1, GL_FALSE, glm::value_ptr(stack->topMatrix()));

    // Draw shape
    shape->draw(*vPosBind, *vNormBind, *vTexCoordsBind);
    
    // Unbind textures
    glActiveTexture(GL_TEXTURE0 + 2);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    stack->popMatrix();
}
