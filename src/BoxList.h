#ifndef BOXLIST_H
#define BOXLIST_H

#ifdef __APPLE__
#include <GLUT/glut.h>
#endif
#ifdef __unix__
#include <GL/glut.h>
#endif
#include "GLSL.h"
#include "MatrixStack.h"

typedef struct BoxNode_t {
   glm::vec3 low;
   glm::vec3 high;
   struct BoxNode_t *next;
} BoxNode;

void freeList(BoxNode *node);

BoxNode* getNode(glm::vec3 low, glm::vec3 high);

#endif
