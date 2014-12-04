#ifndef OCTREE_H
#define OCTREE_H

#include "Mass.h"

#include <stack>

// the indices for being lower/higher than the XYZ value
#define LLL 0
#define LLH 1
#define LHL 2
#define LHH 3
#define HLL 4
#define HLH 5
#define HHL 6
#define HHH 7
#define TOT 8
#define XHI 4
#define YHI 2
#define ZHI 1

class Octree {
private:
   static const float THETA;

   Octree **children;
   const Octree *parent;

   glm::vec3 low;
   glm::vec3 high;
   glm::vec3 middle;

   glm::vec3 centerOfMass;
   float totalMass;

   const Mass *mass; // a leaf node holds either 0 or 1 Masses

   Octree(const glm::vec3 &low, const glm::vec3 &high, const Octree *parent);

   Octree* makeChild(int child) const;

   int pickChild(const glm::vec3 &pos) const;

   void insertChild(const Mass *m);

public:
   Octree(const glm::vec3 &low, const glm::vec3 &high);

   ~Octree();

   void addMass(const Mass *m);

   void calcCOM();

   glm::vec3 getCOM() const;

   float getMass() const;

   bool isLeaf() const;

   Octree* getChild(int ndx) const;

   const Octree* getParent() const;
};

class OctreeIterator {
private:
   const Octree *current;
   std::stack<int> indices;
   int currentIndex;

public:
   OctreeIterator(const Octree *tree);

   void next();

   void nextSkipChildren();

   const Octree *get() const;

   bool atEnd() const;
};

#endif
