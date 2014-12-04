#include "Mass.h"
#include "Octree.h"

#include "glm/gtx/string_cast.hpp"

#include <iostream>

#define THETA 0.5f

int main() {
   Octree tree(glm::vec3(0.0f), glm::vec3(100.0f));

   tree.addMass(new Mass(glm::vec3(25.0f, 40.0f, 0.0f), 1));
   tree.addMass(new Mass(glm::vec3(76.0f,  2.0f, 0.0f), 2));
   tree.addMass(new Mass(glm::vec3(76.0f, 15.0f, 0.0f), 3));
   tree.addMass(new Mass(glm::vec3(88.0f, 15.0f, 0.0f), 4));
   tree.addMass(new Mass(glm::vec3(55.0f, 90.0f, 0.0f), 5));

   tree.calcCOM();

   OctreeIterator iter(&tree);
   while (! iter.atEnd()) {
      const Octree *o = iter.get();
      if (o->isLeaf()) {
         std::cout << "Calculating force for " << glm::to_string(iter.get()->getCOM()) << std::endl;

         OctreeIterator iter2(&tree);
         while (! iter2.atEnd()) {
            const Octree *t = iter2.get();

            if (o == t) {
               iter2.next();
            } else if (t->isLeaf()) {
               std::cout << "Adding force from leaf  " << glm::to_string(t->getCOM()) << std::endl;
               iter2.next();
            } else {
               float s = t->getWidth();
               glm::vec3 dist = o->getCOM() - t->getCOM();

               // if width / distance > THETA -> width*width / distance*distance > THETA * THETA
               if (s*s / glm::dot(dist, dist) > THETA*THETA) {
                  iter2.next();
               } else {
                  std::cout << "Adding force from group " << glm::to_string(t->getCOM()) << std::endl;
                  iter2.nextSkipChildren();
               }
            }   
         }

         std::cout << std::endl;
      }

      iter.next();
   }
}
