#include "Simulator.h"

#include <sys/time.h>
#include <limits>
#include <algorithm>

#define G 6.67384E-4//11
#define THETA 0.5

static time_t firstTime = 0;

static float getTime() {
   struct timeval tp;
   gettimeofday(&tp, NULL);
   if (firstTime == 0) {
      firstTime = tp.tv_sec;
   }
   return tp.tv_sec - firstTime + tp.tv_usec / 1000000.0f;
}

static int getIndex(thread_mass_arg_t *args) {
   pthread_mutex_lock(args->posMut);
   int ndx = args->pos++;
   pthread_mutex_unlock(args->posMut);
   return ndx;
}

static const Octree* getTree(barnes_hut_t *args) {
   const Octree *tree = NULL;
   pthread_mutex_lock(args->posMut);

   while (! args->iter->atEnd()) {
      const Octree *tmp = args->iter->get();
      args->iter->next();

      if (tmp->isLeaf()) {
         tree = tmp;
         break;
      }
   }

   pthread_mutex_unlock(args->posMut);
   return tree;
}

Mass* Simulator::selectedMass;

void *updateMass(void *p) {
   thread_mass_arg_t *args = (thread_mass_arg_t*) p;
   std::vector<Mass*> *masses = args->masses;

   while (args->running) {
      pthread_barrier_wait(args->startBarrier);
      const int count = masses->size();

      int ndx = getIndex(args);
      while (ndx < count) {
         Mass *i = masses->at(ndx);
         for (int ndx2 = ndx + 1; ndx2 < count; ++ndx2) {
            Mass *j = masses->at(ndx2);

            float force = G * i->getMass() * j->getMass() / i->squaredDist(*j);
            glm::vec3 direction = i->getDirection(*j);
            j->addForce(force, direction);
            i->addForce(-force, direction);
         }

         ndx = getIndex(args);
      }

      pthread_barrier_wait(args->endBarrier);
   }

   return NULL;
}

void *updateBarnesHut(void *p) {
   barnes_hut_t *args = (barnes_hut_t*) p;

   while (args->running) {
      pthread_barrier_wait(args->startBarrier);

      const Octree *o = getTree(args);
      while (o) {
         OctreeIterator iter(args->tree);

         while (! iter.atEnd()) {
            const Octree *t = iter.get();
            glm::vec3 d = t->getCOM() - o->getCOM();
            float distance = glm::dot(d, d);

            if (o == t) {
               iter.next();
            } else if (t->isLeaf()) {
               float force = G * o->getMass() * t->getMass() / distance;
               o->addForce(force, glm::normalize(d));

               iter.next();
            } else {
               float s = t->getWidth();

               // if width / distance > THETA -> width*width / distance*distance > THETA * THETA
               if (s*s / distance > THETA*THETA) {
                  iter.next();
               } else {
                  float force = G * o->getMass() * t->getMass() / distance;
                  o->addForce(force, glm::normalize(d));

                  iter.nextSkipChildren();
               }
            }   
         }

         o = getTree(args);
      }

      pthread_barrier_wait(args->endBarrier);
   }

   return NULL;
}

void *update(void *p) {
   thread_arg_t *args = (thread_arg_t*) p;
   std::vector<Mass*> *masses = args->masses;

   pthread_t threads[NUM_THREADS];
   pthread_barrier_t startBarrier;
   pthread_barrier_t endBarrier;
   pthread_mutex_t posMut;

   pthread_barrier_init(&startBarrier, NULL, NUM_THREADS + 1);
   pthread_barrier_init(&endBarrier, NULL, NUM_THREADS + 1);
   pthread_mutex_init(&posMut, NULL);

   if (args->curTime == 0.0f) {
      args->curTime = getTime();
      for (std::vector<Mass*>::iterator iter = masses->begin(); iter != masses->end(); ++iter) {
         (*iter)->stepTime(args->curTime);
      }
   }

   // thread_mass_arg_t tArgs;
   // tArgs.running = true;
   // tArgs.startBarrier = &startBarrier;
   // tArgs.endBarrier = &endBarrier;
   // tArgs.posMut = &posMut;
   // tArgs.masses = args->masses;

   barnes_hut_t bArgs;
   bArgs.running = true;
   bArgs.startBarrier = &startBarrier;
   bArgs.endBarrier = &endBarrier;
   bArgs.posMut = &posMut;

   for (int i = 0; i < NUM_THREADS; i++) {
      // pthread_create(threads + i, NULL, updateMass, (void *) &tArgs);
      pthread_create(threads + i, NULL, updateBarnesHut, (void *) &bArgs);
   }

   while (args->running) {
      pthread_mutex_lock(args->mut);

      glm::vec3 mins(std::numeric_limits<float>::max());
      glm::vec3 maxs(-std::numeric_limits<float>::max());

      for (std::vector<Mass*>::iterator i = masses->begin(); i != masses->end(); ++i) {
         glm::vec3 pos = (*i)->getPosition();

         if (pos.x < mins.x)
            mins.x = pos.x;
         if (pos.x > maxs.x)
            maxs.x = pos.x;

         if (pos.y < mins.y)
            mins.y = pos.y;
         if (pos.y > maxs.y)
            maxs.y = pos.y;

         if (pos.z < mins.z)
            mins.z = pos.z;
         if (pos.z > maxs.z)
            maxs.z = pos.z;


         // for (std::vector<Mass*>::iterator j = i + 1; j != masses->end(); ++j) {
         //    float d = (*i)->getRadius() + (*j)->getRadius();
         //    if ((*i)->squaredDist(**j) <= d*d) {

         //       float m_t = (*i)->getMass() + (*j)->getMass();
         //       float p1 = (*i)->getMass() / m_t;
         //       float p2 = (*j)->getMass() / m_t;

         //       Mass *combined = new Mass(p1 * (*i)->getPosition() + p2 * (*j)->getPosition(),
         //                                 p1 * (*i)->getVelocity() + p2 * (*j)->getVelocity(),
         //                                 m_t,
         //                                 args->curTime);

         //       std::vector<Mass*>::iterator last = i - 1;

         //       delete *i;
         //       delete *j;
         //       masses->erase(j);
         //       masses->erase(i);
         //       masses->push_back(combined);

         //       i = last;
         //       break;
         //    }
         // }
      }

      Octree *tree = new Octree(mins, maxs);
      std::vector<int> toRemove;
      for (std::vector<Mass*>::iterator i = masses->begin(); i != masses->end(); ++i) {
         Mass *m = tree->findCollision(*i);
         if (!m) {
            (*i)->setIndex(i - masses->begin());
            tree->addMass(*i);
         } else {
            tree->removeMass(m);
            toRemove.push_back(m->getIndex());

            float m_t = (*i)->getMass() + m->getMass();
            float p1 = (*i)->getMass() / m_t;
            float p2 = m->getMass() / m_t;

            Mass *combined = new Mass(p1 * (*i)->getPosition() + p2 * m->getPosition(),
                                      p1 * (*i)->getVelocity() + p2 * m->getVelocity(),
                                      m_t,
                                      args->curTime);
            combined->setDrawable((p1 > p2 ? (*i) : m)->getDrawable());
            std::vector<Mass*>::iterator last = i - 1;
            delete *i;
            masses->erase(i);

            masses->push_back(combined);
            Simulator::setSelectedMass(combined);

            i = last;
         }
      }

      std::sort(toRemove.begin(), toRemove.end());
      for (std::vector<int>::reverse_iterator rit = toRemove.rbegin(); rit!= toRemove.rend(); ++rit) {
         delete *(masses->begin() + *rit);
         masses->erase(masses->begin() + *rit);
      }

      // tArgs.pos = 0;
      bArgs.tree = tree; /*new Octree(mins, maxs);
      for (std::vector<Mass*>::iterator i = masses->begin(); i != masses->end(); ++i) {
         bArgs.tree->addMass(*i);
      }*/
      bArgs.tree->calcCOM();
      bArgs.iter = new OctreeIterator(bArgs.tree);

      pthread_barrier_wait(&startBarrier);
      if (args->treeList) {
         freeList(args->treeList);
      }
      args->treeList = bArgs.tree->getLeafList();
      pthread_barrier_wait(&endBarrier);

      delete bArgs.iter;
      delete bArgs.tree;

      args->curTime = getTime();

      for (std::vector<Mass*>::iterator iter = masses->begin(); iter != masses->end(); ++iter) {
         (*iter)->stepTime(args->curTime);
      }

      pthread_mutex_unlock(args->mut);
   }
   // tArgs.running = false;
   bArgs.running = false;
   pthread_barrier_destroy(&startBarrier);
   pthread_barrier_destroy(&endBarrier);
   pthread_mutex_destroy(&posMut);

   for (int i = 0; i < NUM_THREADS; i++) {
      pthread_join(threads[i], NULL);
   }

   return NULL;
}

Simulator::Simulator() {
   pthread_mutex_init(&mut, NULL);
   threadArgs.masses = &masses;
   threadArgs.mut = &mut;
   threadArgs.running = true;
   threadArgs.curTime = 0.0f;
   threadArgs.treeList = NULL;
}

Simulator::~Simulator() {
   pthread_mutex_destroy(&mut);
}

Mass Simulator::getSelectedMass() {
   return *selectedMass;
}

void Simulator::nextMass(Simulator* s) {
   selectedMass = s->getMasses()->at(rand() % s->getMasses()->size());
}

std::vector<Mass*> *Simulator::getMasses() {
   return &masses;
}

void Simulator::setSelectedMass(Mass *mass) {
   selectedMass = mass;
}

void Simulator::addMass(Mass *mass) {
   masses.push_back(mass);
}

void Simulator::start() {
   pthread_create(&thread, NULL, update, (void *) &threadArgs);
}

void Simulator::pause() {
   pthread_mutex_lock(&mut);
}

void Simulator::resume() {
   pthread_mutex_unlock(&mut);
}

void Simulator::stop() {
   threadArgs.running = false;
   pthread_join(thread, NULL);
}

BoxNode* Simulator::getOctree() {
   BoxNode *l = threadArgs.treeList;
   threadArgs.treeList = NULL;
   return l;
}
