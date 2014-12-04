#include "Octree.h"

const float Octree::THETA = 0.5f;

Octree::Octree(const glm::vec3 &low, const glm::vec3 &high) {
   children = NULL;
   parent = NULL;

   this->low = low;
   this->high = high;
   this->middle = 0.5f * low + 0.5f * high;

   centerOfMass = glm::vec3(0.0f);
   totalMass = 0.0f;

   mass = NULL;
}

Octree::Octree(const glm::vec3 &low, const glm::vec3 &high, const Octree *parent) {
   children = NULL;
   this->parent = parent;

   this->low = low;
   this->high = high;
   this->middle = 0.5f * low + 0.5f * high;

   centerOfMass = glm::vec3(0.0f);
   totalMass = 0.0f;

   mass = NULL;
}

Octree::~Octree() {
   if (children) {
      for (int i = 0; i < TOT; i++) {
         if (children[i]) {
            delete children[i];
         }
      }

      delete[] children;
   }
}

Octree* Octree::makeChild(int child) const {
   glm::vec3 l;
   glm::vec3 h;

   if (child & XHI) {
      l.x = middle.x;
      h.x = high.x;
   } else {
      l.x = low.x;
      h.x = middle.x;
   }

   if (child & YHI) {
      l.y = middle.y;
      h.y = high.y;
   } else {
      l.y = low.y;
      h.y = middle.y;
   }

   if (child & ZHI) {
      l.z = middle.z;
      h.z = high.z;
   } else {
      l.z = low.z;
      h.z = middle.z;
   }

   return new Octree(l, h, this);
}

int Octree::pickChild(const glm::vec3 &pos) const {
   int child = 0;
   if (pos.x >= middle.x)
      child += XHI;
   if (pos.y >= middle.y)
      child += YHI;
   if (pos.z >= middle.z)
      child += ZHI;
   return child;
}

void Octree::insertChild(const Mass *m) {
   int child = pickChild(m->getPosition());
   if (! children[child]) {
      children[child] = makeChild(child);
   }
   children[child]->addMass(m);
}

void Octree::addMass(const Mass *m) {
   totalMass += m->getMass();
   centerOfMass += m->getMass() * m->getPosition();

   if (!mass && !children) {
      mass = m;
   } else if (children) {
      insertChild(m);
   } else {
      children = new Octree*[TOT];
      insertChild(mass);
      insertChild(m);
      mass = NULL;
   }
}

void Octree::calcCOM() {
   centerOfMass /= totalMass;
   if (children) {
      for (int i = 0; i < TOT; i++) {
         if (children[i]) {
            children[i]->calcCOM();
         }
      }
   }
}

glm::vec3 Octree::getCOM() const {
   return centerOfMass;
}

float Octree::getMass() const {
   return totalMass;
}

bool Octree::isLeaf() const {
   return !children;
}

Octree* Octree::getChild(int ndx) const {
   return children ? children[ndx] : NULL;
}

const Octree* Octree::getParent() const {
   return parent;
}

OctreeIterator::OctreeIterator(const Octree *tree) {
   current = tree;
   currentIndex = -1;
}

void OctreeIterator::next() {
   if (! current)
      return;

   Octree *child;
   for (++currentIndex; currentIndex < TOT; currentIndex++) {
      if ((child = current->getChild(currentIndex))) {
         indices.push(currentIndex);
         current = child;
         currentIndex = -1;
         return;
      }
   }

   if ((current = current->getParent())) {
      currentIndex = indices.top();
      indices.pop();
      next();
   }
}

void OctreeIterator::nextSkipChildren() {
   if (! current)
      return;

   if ((current = current->getParent())) {
      currentIndex = indices.top();
      indices.pop();
      next();
   }
}

const Octree* OctreeIterator::get() const {
   return current;
}

bool OctreeIterator::atEnd() const {
   return !current;
}
