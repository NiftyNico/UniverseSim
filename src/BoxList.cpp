#include "BoxList.h"

static BoxNode *freeNodes = NULL;
static BoxNode *endNode = NULL;

void freeList(BoxNode *node) {
   if (endNode) {
      endNode->next = node;
   } else {
      freeNodes = node;
      for (endNode = freeNodes; endNode->next; endNode = endNode->next)
         ;
   }
}

BoxNode* getNode(glm::vec3 low, glm::vec3 high) {
   BoxNode *node;

   if (freeNodes) {
      node = freeNodes;

      if (! (freeNodes = freeNodes->next)) {
         endNode = NULL;
      }
   } else {
      node = (BoxNode *) malloc(sizeof(BoxNode));
   }

   node->low = low;
   node->high = high;
   node->next = NULL;

   return node;
}
