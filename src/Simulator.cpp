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

static int getIndex(thread_mass_arg_t *args) {
   pthread_mutex_lock(args->posMut);
   int ndx = args->pos++;
   pthread_mutex_unlock(args->posMut);
   return ndx;
}

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

   thread_mass_arg_t tArgs;
   tArgs.running = true;
   tArgs.startBarrier = &startBarrier;
   tArgs.endBarrier = &endBarrier;
   tArgs.posMut = &posMut;
   tArgs.masses = args->masses;

   for (int i = 0; i < NUM_THREADS; i++) {
      pthread_create(threads + i, NULL, updateMass, (void *) &tArgs);
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

                  float m_t = (*i)->getMass() + (*j)->getMass();
                  float p1 = (*i)->getMass() / m_t;
                  float p2 = (*j)->getMass() / m_t;

                  Mass *combined = new Mass(p1 * (*i)->getPosition() + p2 * (*j)->getPosition(),
                                            p1 * (*i)->getVelocity() + p2 * (*j)->getVelocity(),
                                            m_t,
                                            args->curTime);

                  // don't worry about optimizing the vector removal anymore...
                  //Mass *combined = new Mass(0.5f * ((*i)->getPosition() + (*j)->getPosition()), ((*i)->getMass() * (*i)->getVelocity() + (*j)->getMass() * (*j)->getVelocity()) / (*i)->getMass() + (*j)->getMass(), (*i)->getMass() + (*j)->getMass(), args->curTime);
                  //masses->at(i - masses->begin()) = combined;
                  //masses->at(j - masses->begin()) = masses->back();
                  //masses->pop_back();

                  delete *i;
                  delete *j;
                  masses->erase(j);
                  masses->erase(i);
                  masses->push_back(combined);

                  change = true;
                  goto endloop;
               }
            }
         }
         endloop:
         ; // there needs to be a statement here
      }

      tArgs.pos = 0;
      pthread_barrier_wait(&startBarrier);
      pthread_barrier_wait(&endBarrier);

      args->curTime = getTime();

      for (std::vector<Mass*>::iterator iter = masses->begin(); iter != masses->end(); ++iter) {
         (*iter)->stepTime(args->curTime);
      }

      pthread_mutex_unlock(args->mut);
   }
   tArgs.running = false;
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
