/*
 * Author: Nathaniel Rupprecht
 * Start Data: January 24, 2018
 *
 */

#ifndef __SIM_DATA_BASE_HPP__
#define __SIM_DATA_BASE_HPP__

// Includes
#include "../../include/Utility.hpp"
#include "../../include/Bounds.hpp"
#include "../../include/aligned_array.hpp"
#include "../objects/Particle.hpp"
#include "../objects/Wall.hpp"
#include "../objects/Characteristic.hpp"

namespace GFlow {

  // Forward declaration to DataRecord
  class DataRecord;
  // Forward declaration to ExternalForce
  class ExternalForce;
  // Forward declaration to Creator
  class Creator;
  // Forward declaration to FileParser
  class FileParser;
  // Forward declaration to Sectorization
  class Sectorization;
  // Forward declaration to ForceHandler
  class ForceHandler;

  /*
   * @class SimData
   *
   */
  template<typename Derived>
  class SimDataBase {
  public:
    // Constructor taking domain and simulation dimensions
    SimDataBase(const Bounds&, const Bounds&);

    // Destructor
    ~SimDataBase();

    // Reserve particle size
    void reserve(int, int=-1);

    // Reserve additional space for particles
    void reserveAdditional(int, int=-1);

    // Add walls
    void addWall(const Wall&);
    void addWall(const Bounds&);
    void addWall(const vector<Wall>&);

    // Add particles
    int addParticle(const Particle&);
    void addParticle(const vector<Particle>&);
    int addParticle(const Particle&, Characteristic*);

    // Remove particles
    void removeAt(int);

    // Access sizes and capacities
    int getDomainSize()     { return domain_size; }
    int getDomainEnd()      { return domain_end; }
    int getDomainCapacity() { return domain_capacity; }
    int getEdgeSize()       { return edge_size; }
    int getEdgeCapacity()   { return edge_capacity; }

    // Returns the bounds
    Bounds getBounds()    { return bounds; }
    Bounds getSimBounds() { return simBounds; } 

    // Return wrapping
    bool getWrapX()      { return wrapX; }
    bool getWrapY()      { return wrapY; }

    // Get a copy of a particle
    Particle makeParticle(int);

    // Get position record
    // vec2* getPRPtr()     { return positionRecord.getPtr(); }

    // Get pressure data
    void getPressureData(vector<PData>&, RealType=0.);

    // Get walls
    vector<Wall>& getWalls() { return walls; }

    // Get a list of all the particles
    vector<Particle> getParticles();

    // Get all the particles withing a cutoff radius
    vector<Particle> getParticles(vec2, RealType);
    vector<int> getParticlesID(vec2, RealType);

    // Get the characteristics
    auto& getCharacteristics() { return characteristics; }

    // Get the force handler
    ForceHandler* getForceHandler() { return forceHandler; }

    // Get the external forces
    vector<ExternalForce*>& getExternalForces() { return externalForces; }

    // Get termination indicator
    bool getTerminate() { return terminate; }

    // Wrap positions and angles
    void wrap(RealType&, RealType&); // Position wrapping
    void wrap(RealType&);            // Angle wrapping

    // Displacement functions
    vec2 getDisplacement(const RealType, const RealType, const RealType, const RealType);
    vec2 getDisplacement(const vec2, const vec2);
    vec2 getWallDisplacement(const Wall&, const vec2, RealType);

    // Calculate packing fraction
    RealType getPhi();

    // Get the indices of the two closest particles to a particular particle
    pair<int, int> getClosestTwo(int);

#if USE_MPI == 1 // Get MPI data
    int getRank()    { return rank; }
    int getNumProc() { return numProc; }

    void atomMove();
    void atomCopy();
#endif

    /** Mutators **/

    // Set wrapping boundary conditions
    void setWrap(bool b)  { wrapX = wrapY = b; }
    void setWrapX(bool b) { wrapX = b; }
    void setWrapY(bool b) { wrapY = b; }

    // Set bounds
    void setBounds(const Bounds& b)    { bounds = b; }
    void setSimBounds(const Bounds& b) { simBounds = b; }

    // Set sectorization
    void setSectors(Sectorization *sec) { sectors = sec; }

    // Set force handler
    void setForceHandler(ForceHandler* frc) { forceHandler = frc; }

    // Add an external force or remove all of them
    void addExternalForce(ExternalForce*);
    void clearExternalForces();

    // Clear forces and torques
    void clearValues();

    // Update position record
    void updatePositionRecord();

    // Set initial position vector
    void setInitialPositions();

    // Get initial position vector
    const vector<vec2>& getInitialPositions() const;

    // Remove particles that overlap too much
    void removeOverlapping(RealType=0.03);

    // Set termination indicator
    void setTerminate(bool s) { terminate = s; }

    // DataRecord is a friend class
    friend class DataRecord;
    // Creator is a friend class
    friend class Creator;
    // FileParser is a friend class
    friend class FileParser;

    struct BadParticle {
      BadParticle(int i) : index(i) {};
      int index;
    };
    
  protected:
    // CRTP derived function
    Derived& derived();

    // Constant CRTP derived function
    Derived const& derived() const;

    // Private helper functions
    inline void compressArrays();

    // Number of domain particles
    int domain_size;

    // Position after the last valid particle in the domain
    int domain_end;

    // Storage allocated for main particles
    int domain_capacity;

    // Number of edge particles
    int edge_size;
    
    // Storage allocated for edge particles
    int edge_capacity;

    // Wall data
    vector<Wall> walls;

    // Holes in the arrays
    list<int> holes;

    // Characteristics - we are in charge of managing (deleting) these
    std::map<int, Characteristic*> characteristics;

    // Domain and Simulation bounds
    Bounds bounds;
    Bounds simBounds;

    // External forces
    vector<ExternalForce*> externalForces;

    // Simulation boundary conditions
    bool wrapX, wrapY;

    // The sectorization that handles this data
    Sectorization *sectors;

    // The force handler that handles our forces
    ForceHandler *forceHandler;

    // A termination indicator
    bool terminate;

#ifdef USE_MPI // Rank and number of processors - for MPI
    int rank, numProc;
#endif
    
  };

#include "SimDataBase.cpp"

}
#endif // __SIM_DATA_HPP__
