#include "Simulator.h"

#include <sys/time.h>

#define G 6.67384E-2//11

static time_t firstTime = 0;

static float getTime() {
   struct timeval tp;
   gettimeofday(&tp, NULL);
   if (firstTime == 0) {
      firstTime = tp.tv_sec;
   }
   return tp.tv_sec - firstTime + tp.tv_usec / 1000000.0f;
}

void *update(void *p) {
   thread_arg_t *args = (thread_arg_t*) p;
   std::vector<Mass*> *masses = args->masses;
   float curTime = *(args->curTime);

   if (curTime == 0.0f) {
      curTime = getTime();
      for (std::vector<Mass*>::iterator iter = masses->begin(); iter != masses->end(); ++iter) {
         (*iter)->stepTime(curTime);
      }
   }

   while (args->running) {
      pthread_mutex_lock(args->mut);

      bool change = true;
      while (change) {
         change = false;
         for (std::vector<Mass*>::iterator i = masses->begin(); i != masses->end(); ++i) {
            for (std::vector<Mass*>::iterator j = i + 1; j != masses->end(); ++j) {
               float d = (*i)->getRadius() + (*j)->getRadius();
               if ((*i)->squaredDist(**j) <= d*d) {
                  Mass *combined = new Mass(0.5f * ((*i)->getPosition() + (*j)->getPosition()), ((*i)->getMass() * (*i)->getVelocity() + (*j)->getMass() * (*j)->getVelocity()) / (*i)->getMass() + (*j)->getMass(), (*i)->getMass() + (*j)->getMass(), curTime);
                  masses->at(i - masses->begin()) = combined;
                  masses->at(j - masses->begin()) = masses->back();
                  masses->pop_back();

                  change = true;
                  //i = j = masses->end();
                  goto endloop;
               }
            }
         }
         endloop:
         ; // there needs to be a statement here
      }

      for (std::vector<Mass*>::iterator i = masses->begin(); i != masses->end(); ++i) {
         for (std::vector<Mass*>::iterator j = i + 1; j != masses->end(); ++j) {
            float force = G * (*i)->getMass() * (*j)->getMass() / (*i)->squaredDist(**j);
            glm::vec3 direction = (*i)->getDirection(**j);
            (*j)->addForce(force, direction);
            (*i)->addForce(-force, direction);
         }
      }

      curTime = getTime();
      *(args->curTime) = curTime;

      for (std::vector<Mass*>::iterator iter = masses->begin(); iter != masses->end(); ++iter) {
         (*iter)->stepTime(curTime);
      }

      pthread_mutex_unlock(args->mut);
   }

   return NULL;
}

Simulator::Simulator() {
   curTime = 0.0f;
   pthread_mutex_init(&mut, NULL);
   threadArgs.masses = &masses;
   threadArgs.mut = &mut;
   threadArgs.running = true;
   threadArgs.curTime = &curTime;
}

Simulator::~Simulator() {
   pthread_mutex_destroy(&mut);
}

std::vector<Mass*> *Simulator::getMasses() {
   return &masses;
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
