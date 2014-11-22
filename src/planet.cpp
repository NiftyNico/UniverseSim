#include "planet.h"

MatrixStack* Planet::stack;
GLint* Planet::stackBind;
GLint* Planet::colorBind;
GLint* Planet::reflectBind;
GLint* Planet::atmosBind;
GLint* Planet::vPosBind;
GLint* Planet::vNormBind;
GLint* Planet::vTexCoordsBind;
GLint* Planet::texTransBind;

Planet::Planet(Shape* shape, GLuint* coloring, GLuint* reflectivity, GLuint* atmosphere) {
    Planet::shape = shape;
    Planet::coloring = coloring;
    Planet::reflectivity = reflectivity;
    Planet::atmosphere = atmosphere;
    transpose[0][0] = 1;
    transpose[1][1] = 1;
    transpose[2][2] = 1;
    transpose[3][3] = 1;
}

void Planet::setup(MatrixStack* stack, GLint* stackBind, GLint* colorBind, GLint* reflectBind, GLint* atmosBind, 
 GLint* vPosBind, GLint* vNormBind, GLint* vTexCoordsBind, GLint* texTransBind) {
    Planet::stack = stack;
    Planet::stackBind = stackBind;
    Planet::colorBind = colorBind;
    Planet::reflectBind = reflectBind;
    Planet::atmosBind = atmosBind;
    Planet::vPosBind = vPosBind;
    Planet::vNormBind = vNormBind;
    Planet::vTexCoordsBind = vTexCoordsBind;
    Planet::texTransBind = texTransBind;
}

void Planet::rotate(float angle, glm::vec3 rotateVec) {
    transpose *= glm::rotate(angle, rotateVec);
}

void Planet::translate(glm::vec3 transVec) {
    transpose *= glm::translate(transVec);
}

void Planet::scale(glm::vec3 scaleVec) {
    transpose *= glm::scale(scaleVec);
}

void Planet::draw() {
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

    glm::mat3 cloudMover(1.0);
    //T.translate(glm::vec3(moveCloudsBy, 0.0f, 0.0f)); 
    cloudMover[0][0] = 1.0f;
    cloudMover[0][1] = 0.0f;
    cloudMover[0][2] = 0.0f;
    cloudMover[1][0] = 0.0f;
    cloudMover[1][1] = 1.0f;
    cloudMover[1][2] = 0.0f;
    cloudMover[2][0] = 0.05f;
    cloudMover[2][1] = 0.0f;
    cloudMover[2][2] = 1.0f;

    // Texture matrix
    // We're sending in GL_TRUE for the transpose argument, because we want
    // the translation to be the right-most column instead of the bottom row.
    glUniformMatrix3fv(*texTransBind, 1, GL_FALSE, glm::value_ptr(cloudMover));

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