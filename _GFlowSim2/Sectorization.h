#ifndef SECTORIZATION_H
#define SECTORIZATION_H

#include "Interactions.h"
#include "Characteristic.h"
#include "StatFunc.h"
#include <mpi.h>
#include <stdlib.h> // For aligned_malloc

// Safe free
template<typename T> inline void zfree(T* &ptr) {
  if (ptr) free(ptr);
  ptr = 0;
}

// To use:
// --- These steps can be done in any order ---
// 1) Set domain and simulation bounds
// 2) Set characteristic length and cutoff
// 3) Set parameters like gravity and whether to do interactions
// 4) Add particles and walls
// 5) Set expected number of particles
// --- This step must be last ---
// 6) Call "initialize"
// --- Ready to run ---
class Sectorization {
 public:
  Sectorization();
  ~Sectorization();

  void initialize();        // Initialize the sectors, can pass in a cutoff

  // Accessors
  int getNSX()                   { return nsx; }
  int getNSY()                   { return nsy; }
  int getSize()                  { return size; }
  int getASize()                 { return asize; }
  int getESize()                 { return esize; }
  int getArrayEnd()              { return array_end; }
  int getWallSize()              { return walls.size(); }
  double getSecWidth()           { return secWidth; }
  double getSecHeight()          { return secHeight; }
  double getEpsilon()            { return epsilon; }
  double getMaxNeighborDiff()    { return maxNLDiff; }
  double getTransferTime()       { return transferTime; }
  double getCutoff()             { return cutoff; }
  bool getDoInteractions()       { return doInteractions; }
  bool getWrapX()                { return wrapX; }
  bool getWrapY()                { return wrapY; }
  vec2 getGravity()              { return gravity; }
  vector<Particle>& getParticles();
  Bounds getBounds()             { return bounds; }
  Bounds getSimBounds()          { return simBounds; }
  bool isFull()                  { return asize<=array_end; }
  bool isValid(int i)            { return -1<it[i]; }

  // Pointer accessors
  double *getPX()                { return px; }
  double *getPY()                { return py; }
  double *getSG()                { return sg; }
  int *getIT()                   { return it; }
  Characteristic** getCH()       { return ch; }

  // Statistic accessors
  int getNeighborListSize()      { return neighborList.size(); }
  double getAvePerNeighborList();

  // Mutators
  void giveDomainInfo(int x, int y) { ndx=x; ndy=y; }
  void setEpsilon(double e)      { epsilon = e; sqrtEpsilon = sqrt(e); }
  void setDoInteractions(bool i) { doInteractions = i; }
  void setWrapX(bool w)          { wrapX = w; }
  void setWrapY(bool w)          { wrapY = w; }
  void setWrap(bool w)           { wrapX = wrapY = w; }
  void setDrag(bool d)           { drag = d; }
  void setGravity(vec2 g)        { gravity = g; }
  void setTemperature(double t)  { temperature = t; DT1 = t/(6*viscosity*PI); }
  void setViscosity(double h)    { viscosity = h; DT1 = temperature/(6*h*PI); }
  void setCoeff(double c);
  void setSigma(double r);
  void setCutoff(double c)       { cutoff = c; }
  void setSkinDepth(double d)    { skinDepth = d; }
  void setBounds(double, double, double, double);
  void setBounds(Bounds);
  void setSimBounds(double, double, double, double);
  void setSimBounds(Bounds);
  void setUseCharacteristics(bool);
  void setInteractionType(int);
  void setASize(int i);
  void stopParticles();
  void setCommWork(MPI::Intercomm &comm) { CommWork = comm; }
  void resetComm()               { CommWork = MPI::COMM_WORLD; }
  void discard();
  void discardAll();
  void updateList(); // Fill plist with the particles from the data buffers

  // Functionality
  void update();                     // Do a timestep
  void updateSectors();              // Update the sectors, migrating particles to the correct sectors

  // Addition
  void addParticle(Particle);
  void insertParticle(Particle);
  void insertParticle(Particle, Characteristic*);
  void removeAt(int);
  void addWall(Wall);
  void setCharacteristic(Characteristic*);

  // Statistics
  string printSectors();
  pair<double, int> doStatFunction(StatFunc);
  vector<PData> forceData(int=0, int=0, bool=true);

  // Exception classes
  class BadParticle {};
  class OutOfBounds {};

 private:
  /// Helper functions
  inline void firstHalfKick();
  inline void particleInteractions(); // Handle the interactions between pair of particles
  inline void interactionHelper(int, int, int, double&, double&, bool=true); // Switch statement for particle interaction
  inline void forceChoice(int, const double, const double, double&);
  inline void wallInteractions(); // Handle the interactions between particles and walls
  inline void secondHalfKick();
  inline void wrap(double&, double&);
  inline void wrap(vec2&);           // Keep a position in bounds by wrapping
  inline void wrap(double&);         // Keep an angle between 0 and 2*PI
  inline int  getSec(const vec2&);   // What sector does a position fall into
  inline int  getSec(const double, const double);
  inline void createNeighborLists(bool=false); // Create neighbor lists
  inline void createEdgeNeighborLists();       // Create edge neighbor lists
  inline void createWallNeighborList();
  inline vec2 getDisplacement(vec2, vec2);
  inline vec2 getDisplacement(double, double, double, double);
  inline void makeSectors();
  inline void createArrays(); // Initialize the array and point the pointers to their propper sections
  inline void zeroPointers(); // Zero all pointers. Do not delete, just set them to zero
  inline void remakeParticles(); // Delete and reallocate particle data arrays
  inline void setParticles();       // Set particle data arrays from plist data
  inline void atom_move();
  inline void passParticles(int, int, const list<int>&, bool=false);
  inline void passParticleSend(const int, const list<int>&, bool=false);
  inline void passParticleRecv(const int, bool=false);
  inline void compressArrays();
  inline void atom_copy();
  inline void memory_rearrange();

  /// Data
  int nsx, nsy;                      // Number of sectors in x and y, includes edge sectors
  int ndx, ndy;                      // Number of domains in x and y
  double secWidth, secHeight;        // The width and height of sectors
  double time;                       // Simulation time
  double epsilon, sqrtEpsilon;       // Timestep and its square root
  double transferTime;               // How much time is spent transfering data
  bool wrapX, wrapY;                 // Whether we wrap distances in the x and y directions
  bool doInteractions;
  bool drag;                         // A drag coefficient, useful for finding a packed solution

  vec2 gravity;                      // Gravitational acceleration
  double temperature, viscosity, DT1;
  double tempDelay, sqrtTempDelay;// How long we wait between applying temperature perturbations
  double lastTemp;                // Last time we applied temperature perturbations

  // All the particles
  vector<Particle> plist;
  vector<Characteristic*> clist;
  vec2 *positionTracker;           
  int size, array_end, asize; // The number of particles, the index after the last particle, the amount of space allocated for domain particles
  int esize, earray_end, easize; // The number of edge particles, the index after the last edge particle, the amount of space allocated for edge particles

  // Particle data - position (2), velocity (2), force (2), theta, omega, torque, sigma, inverse mass, inverse moment of inertia, repulsion, dissipation, coeff of friction
  double *px, *py, *vx, *vy, *fx, *fy, *th, *om, *tq, *sg, *im, *iI, *rp, *ds, *cf;
  // Particle data - pointers to px, py, vx, vy, etc
  double *pdata[15]; 
  // Particle data - interaction type
  int *it;
  // Particle data - characteristics
  Characteristic **ch;
  bool useCharacteristics;
  // Walls 
  list<Wall> walls;
  
  // Sectors and neighbors
  list<int> *sectors;
  list<list<int> > neighborList;     // Neighbor list for domain particles
  list<list<int> > edgeNeighborList; // Neighbor list for edge particles
  double maxNLDiff; // The largest possible distance from the last time we checked
  list<pair<int, list<Wall*> > > wallNeighbors;
  list<int> holes;
  
  bool redoLists;                    // True if we need to remake neighbor lists
  bool doWallNeighbors;              // Create and use wall Neighbor list
  bool remakeToUpdate;               // Totally remake sectors to update them
  double cutoff, skinDepth;          // The particle interaction cutoff (for finding sector size), and the skin depth (for creating neighbor lists)
  double maxCutR, secCutR;           // The first and second greatest cutoff radii
  int itersSinceBuild, buildDelay;   // How many iterations since we last rebuilt the neighbor list, and how many iterations we wait before rebuilding

  // MPI
  int numProc, rank;                 // The number of processors MPI is using and the rank of this processor
  Bounds bounds, simBounds ;         // The physical dimensions of this domain and of the entire simulation space
  MPI::Intercomm CommWork;           // The communicator for the working processors
};

#endif
