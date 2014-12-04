#include "Mass.h"
#include "Octree.h"

#include "glm/gtx/string_cast.hpp"

#include <iostream>

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
      std::cout << iter.get()->isLeaf() << "\t" << glm::to_string(iter.get()->getCOM()) << std::endl;
      iter.next();
   }
}
