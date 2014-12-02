#ifndef SIMULATOR_H
#define SIMULATOR_H

#ifdef __APPLE__
#include <GLUT/glut.h>
#include "pthread-barrier.h"
#endif
#ifdef __unix__
#include <GL/glut.h>
#endif
#include "GLSL.h"
#include "MatrixStack.h"

#include "Mass.h"

#include <pthread.h>
#include <vector>

#define NUM_THREADS 8

typedef struct {
   std::vector<Mass*> *masses;
   pthread_mutex_t *mut;
   bool running;
   float curTime;
} thread_arg_t;

typedef struct {
   bool running;
   pthread_barrier_t *startBarrier;
   pthread_barrier_t *endBarrier;
   pthread_mutex_t *posMut;
   std::vector<Mass*> *masses;
   int pos;
} thread_mass_arg_t;

class Simulator {
private:
   std::vector<Mass*> masses;
   pthread_t thread;
   pthread_mutex_t mut;
   thread_arg_t threadArgs;

public:
   Simulator();

   ~Simulator();

   std::vector<Mass*> *getMasses();

   void addMass(Mass *mass);

   void start();

   void pause();

   void resume();

   void stop();
};

#endif