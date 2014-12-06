#include "Octree.h"

const float Octree::THETA = 0.5f;

// http://stackoverflow.com/a/13717283/3598299
static bool overlaps(glm::vec3 l1, glm::vec3 h1, glm::vec3 l2, glm::vec3 h2) {
   return std::max(l1.x, l2.x) <= std::min(h1.x, h2.x) &&
          std::max(l1.y, l2.y) <= std::min(h1.y, h2.y) &&
          std::max(l1.z, l2.z) <= std::min(h1.z, h2.z);
}

Octree::Octree(const glm::vec3 &low, const glm::vec3 &high) {
   children = NULL;
   parent = NULL;

   this->low = low;
   this->high = high;
   middle = 0.5f * low + 0.5f * high;
   width = std::min(high.x - low.x, std::min(high.y - low.y, high.z - low.z));

   centerOfMass = glm::vec3(0.0f);
   totalMass = 0.0f;

   mass = NULL;
}

Octree::Octree(const glm::vec3 &low, const glm::vec3 &high, const Octree *parent) {
   children = NULL;
   this->parent = parent;

   this->low = low;
   this->high = high;
   middle = 0.5f * low + 0.5f * high;
   width = std::min(high.x - low.x, std::min(high.y - low.y, high.z - low.z));

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

void Octree::insertChild(Mass *m) {
   int child = pickChild(m->getPosition());
   if (! children[child]) {
      children[child] = makeChild(child);
   }
   children[child]->addMass(m);
}

void Octree::addMass(Mass *m) {
   totalMass += m->getMass();
   centerOfMass += m->getMass() * m->getPosition();

   if (!mass && !children) {
      mass = m;
   } else if (children) {
      insertChild(m);
   } else {
      children = new Octree*[TOT]();
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

float Octree::getWidth() const {
   return width;
}

void Octree::addForce(float force, glm::vec3 direction) const {
   if (mass) {
      mass->addForce(force, direction);
   }
}

BoxNode* Octree::getLeafList() const {
   BoxNode *list = NULL;
   BoxNode *last = NULL;
   BoxNode *curr = NULL;
   OctreeIterator iter(this);

   while (! iter.atEnd()) {
      if (iter.get()->isLeaf()) {
         curr = getNode(iter.get()->low, iter.get()->high);

         if (last) {
            last->next = curr;
         } else {
            list = curr;
         }

         last = curr;
      }

      iter.next();
   }

   return list;
}

void Octree::removeMass(Mass *m) {
   if (mass != m) {
      int child = pickChild(m->getPosition());
      if (children && children[child]) {
         bool last = children[child]->mass = m;
         children[child]->removeMass(m);
         if (last) {
            delete children[child];
            children[child] = NULL;
         }
      }
   } else {
      mass = NULL;
      float mMass = m->getMass();
      Octree *o = this;
      while (o) {
         totalMass -= mMass;
         centerOfMass -= m->getPosition() * m->getMass();
         o = (Octree*) o->parent;
      }
   }
}

Mass* Octree::findCollision(Mass *m, float maxRadius) const {
   float d = m->getRadius() + maxRadius;
   glm::vec3 p = m->getPosition();
   glm::vec3 l(p.x - d, p.y - d, p.z - d);
   glm::vec3 h(p.x + d, p.y + d, p.z + d);
   OctreeIterator iter(this);

   while (! iter.atEnd()) {
      const Octree *o = iter.get();

      if (o->mass == m) {
         iter.next();
      } else if (overlaps(l, h, o->low, o->high)) {
         if (o->isLeaf()) {
            if (m->squaredDist(*(o->mass)) <= d*d) {
               return o->mass;
            }
         }
         iter.next();
      } else {
         iter.nextSkipChildren();
      }
   }

   return NULL;
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
