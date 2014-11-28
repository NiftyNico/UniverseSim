#include "Mass.h"

Mass::Mass(glm::vec3 position, float radius) {
   this->position = position;
   this->velocity = glm::vec3(0.0f);
   this->acceleration = glm::vec3(0.0f);
   this->mass = 4.0 * M_PI * radius * radius * radius / 3.0;
   this->radius = radius;
}

Mass::Mass(glm::vec3 position, glm::vec3 velocity, float mass) {
   this->position = position;
   this->velocity = velocity;
   this->acceleration = glm::vec3(0.0f);
   this->mass = mass;
   this->radius = std::pow(3.0 * mass / (4.0 * M_PI), 1/3.);
}

float Mass::getRadius() {
   return radius;
}

float Mass::getMass() {
   return mass;
}

glm::vec3 Mass::getPosition() {
   return position;
}

glm::vec3 Mass::getVelocity() {
   return velocity;
}

float Mass::squaredDist(const Mass &other) const {
   glm::vec3 diff = position - other.position;
   return glm::dot(diff, diff);
}

glm::vec3 Mass::getDirection(const Mass &other) const {
   return glm::normalize(position - other.position);
}

void Mass::addForce(float force, glm::vec3 direction) {
   acceleration += force / mass * direction;
}

void Mass::stepTime(long newTime) {
   float dt = newTime - curTime;
   position += velocity * dt + 0.5f * acceleration * dt * dt;
   velocity += dt * acceleration;
   acceleration = glm::vec3(0.0f);
   curTime = newTime;
}
