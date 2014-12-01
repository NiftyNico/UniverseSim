#ifndef SIMULATOR_H
#define SIMULATOR_H

#ifdef __APPLE__
#include <GLUT/glut.h>
#endif
#ifdef __unix__
#include <GL/glut.h>
#endif
#include "GLSL.h"
#include "MatrixStack.h"

#include "Mass.h"

#include <pthread.h>
#include <vector>

typedef struct {
   std::vector<Mass*> *masses;
   pthread_mutex_t *mut;
   bool running;
   float *curTime;
} thread_arg_t;

class Simulator {
private:
   float curTime;
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
