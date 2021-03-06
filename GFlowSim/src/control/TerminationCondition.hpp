#ifndef __TERMINAL_CONDITION_HPP__
#define __TERMINAL_CONDITION_HPP__

// Includes
#include "../control/SimData.hpp"

namespace GFlow {

  /*
   * @class TerminalCondition
   *
   */
  class TerminationCondition {
  public:
    TerminationCondition(string m);
    virtual bool check(SimData*) = 0;
    string getMessage() { return message; }
  protected:
    string message;
  };

  class OutsideRegion : public TerminationCondition {
  public:
    OutsideRegion();
    virtual bool check(SimData*);
  };

}
#endif // __TERMINAL_CONDITION_HPP__
