/*
 * Author: Nathaniel Rupprecht
 * Start Data: May 17, 2017
 *
 */

#ifndef __REGION_HPP__
#define __REGION_HPP__

// Includes
#include "../../include/Utility.hpp"

namespace GFlow {

  // Forward declaration to DataCreator
  class DataCreator;

  /*
   * @class Region
   * Contains the data for constructing a region full of particles
   *
   */
  struct Region {
    // Bounds
    RealType left, right, bottom, top;
    // Create vec2 type data
    DataCreator *position, *velocity;
    // Create RealType data
    DataCreator *theta, *omega, *sigma, *inertia, *repulsion, *dissipation, *coeff;
    // Create integer data
    DataCreator *interaction;
  };
  
}
#endif // __REGION_HPP__
