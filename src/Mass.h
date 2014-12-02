#ifndef MASS_H
#define MASS_H

#ifdef __APPLE__
#include <GLUT/glut.h>
#endif
#ifdef __unix__
#include <GL/glut.h>
#endif
#include "GLSL.h"
#include "MatrixStack.h"

#include <pthread.h>

class Mass {
private:
   float curTime;
   float mass;
   float radius;
   glm::vec3 position;
   glm::vec3 velocity;
   glm::vec3 acceleration;
   pthread_mutex_t mut;

public:
   Mass(glm::vec3 position, float radius);

   Mass(glm::vec3 position, glm::vec3 velocity, float mass, float curTime);

   ~Mass();

   float getRadius() const;

   float getMass() const;

   glm::vec3 getPosition() const;

   glm::vec3 getVelocity() const;

   float squaredDist(const Mass &other) const;

   glm::vec3 getDirection(const Mass &other) const;

   void addForce(float force, glm::vec3 direction);

   void stepTime(long newTime);
};

#endif