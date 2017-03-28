// Particle.h
// Created March 3, 2017
// Nathaniel Rupprecht

#ifndef PARTICLE_H
#define PARTICLE_H

#include "DefaultConstants.h"

/* 

Each type of particle has to have the same basic parameters, kinetic variables (e.g. position, velocity) and characteristics (e.g. radius, interaction parameters). Behaviors like "active motion" can be functions that are called by the simulator and modify these variables. That way, we don't have to inherit, and (more importantly) all Particles have the same memory footprint. 
   We can define functions that take two particles and compute the force between them and update the particles that they were given symmetrically or asymmetrically.
   The trouble arises when we need additional variables to specify behaviors, like average run time of a run and tumble particle. We can create a class called Characteristic and create children that represent the behaviors of different types of complex particles e.g Brownian walkers, run and tumble particles, etc.

*/

/// Wall structure --- Size: 8 x sizeof(double) bytes 
struct Wall {
  Wall();
  Wall(vec2, vec2);
  Wall(double, double, double, double);
  
  vec2 getLeft()  const { return left; }
  vec2 getRight() const { return left+length*normal; }

  vec2 left;    // Left and right edge of the wall
  double length;         // Length of the wall (store instead of recalculate)
  vec2 normal;         // Normal vector in wall direction (store instead of recalculate)
  double repulsion, dissipation, coeff;
};

/// Particle data structure -- Size: 128 bytes ( 16 x 8 bytes )
struct Particle {
  // Constructor
  Particle();
  Particle(vec2, double);
  Particle(double, double, double);
  // Helper functions
  void setDensity(double);
  // Error class
  class BadMassError {};
  // Kinetic variable
  vec2 position, velocity, force; // Linear kinetic variables
  double theta, omega, torque;  // Angular kinetic variables
  
  int interaction; // Interaction type

  double sigma; // Radius or force cutoff
  double invMass, invII; // Inverses of mass and moment of inertia
  double repulsion, dissipation, coeff;
};

/// Purely abstract base class for characteristics
class Characteristic {
  // Constructor
  Characteristic() : particle(0) {};
  Characteristic(Particle *p) : particle(p) {};
  
  virtual void apply()=0;
 private:
  Particle *particle;
};

#endif
