/*
 * Author: Nathaniel Rupprecht
 * Start Data: May 16, 2017
 *
 */

#ifndef __FILE_PARSER_HPP__
#define __FILE_PARSER_HPP__

// Includes
#include "ParsingTokens.hpp"
#include "../control/SimData.hpp"
#include "../control/StatusObject.hpp"
#include "../forces/ConstantAcceleration.hpp"
#include "../../include/PrintingUtility.hpp"
#include "../../include/ReadingUtility.hpp"

namespace GFlow {
 
  // Forward declaration to Creator
  class Creator;
  // Forward declaration to region
  struct Region;

  /*
   * @class FileParser
   * Turns a configuration file into a simulation data object
   *
   */
  class FileParser {
  public:
    // Default constructor
    FileParser();

    // Destructor
    ~FileParser();

    // Parse a set up file and create a simulation based on it
    SimData* parse(string, unsigned = 0);

    // Load a stored configuration from a file
    SimData* loadFromFile(string);

    // Save sim data to a file
    void saveToFile(SimData*, string);

    // Get the random seed used
    unsigned getSeed() { return randomSeed; }

    // Give a status object
    void setStatusObject(StatusObject *so) { status = so; }

    // Exception classes
    struct FileDoesNotExist {
      FileDoesNotExist(string n) : name(n) {};
      string name;
    };

    struct UnrecognizedToken {
      UnrecognizedToken(string tok) : token(tok) {};
      string token;
    };

  private:
    // Private helper functions
    inline void make_region(std::ifstream&);
    inline void make_wall(std::ifstream&);
    inline void make_particle(std::ifstream&);
    inline void seed_rand();
    inline void seed_rand(unsigned);

    // List of regions to make
    vector<Region> regions;
    // List of individual particles to add
    vector<Particle> particles;
    // List of walls to add
    vector<Wall> walls;
    // Gravity
    vec2 gravity;
    // Wrapping
    bool wrapX, wrapY;
    // Bounds
    Bounds simBounds;

    // The simulation data we are creating - we are not responsible for deleting this since we always create it for (and return it at the end of) the parse function
    SimData* simData;   

    // The creator we use to set up the simulation
    Creator* creator;

    // Status object
    StatusObject* status;

    // What random seed we used
    unsigned randomSeed;
  };
  
}
#endif // __FILE_PARSER_HPP__