#ifndef __SCALAR_FIELD__
#define __SCALAR_FIELD__

#include "DefaultConstants.h"

class ScalarField {
 public:
  ScalarField();
  ScalarField(double, double, double, double);

  // Accessors
  double get(double, double) const; // <-----
  double& at(int, int);
  double at(int, int)  const;
  double lap(int, int) const;
  int getNSX() const { return nsx; }
  int getNSY() const { return nsy; }
  Bounds getBounds() const { return bounds; }
  double getResolution() const { return (bounds.right - bounds.left)/nsx; }
  double getDX() const { return dx; }
  double getDY() const { return dy; }
  double dX(double, double)  const;
  double dY(double, double)  const;
  double dXAt(int, int)  const;
  double dYAt(int, int)  const;
  double d2X(double, double) const;
  double d2Y(double, double) const;
  double d2XAt(int, int) const;
  double d2YAt(int, int) const;
  vector<double> sliceX(double, int=100);
  vector<double> sliceY(double, int=100);
  // double derivative(double, double, int, int) const;
  // double derivativeAt(int, int, int, int) const;
  void laplacian();
  void laplacian(ScalarField&);
  bool empty() { return array==0; }

  // Mutators
  void increase(double, double, double);
  void reduce(double, double, double);
  void propReduceCue(double, double, double); // Load proportional reductions
  void propReduceExec(); // Actually to the proportional reduction
  void set(std::function<double(double,double)>);
  void setBounds(Bounds);
  void setBounds(double, double, double, double);
  void setResolution(double);
  void set(Bounds, double);
  void setDiffusion(double d) { diffusion = d; }
  void setLambda(double l) { lambda = l; }
  void discard();
  void setWrap(bool w)  { wrapX = wrapY = w; }
  void setWrapX(bool w) { wrapX = w; }
  void setWrapY(bool w) { wrapY = w; }

  // Field evolution, pass in timestep
  void update(double);

  // Printing
  friend std::ostream& operator<<(std::ostream&, ScalarField &);
  bool printToCSV(string, int=50);
  
  // Error class
  class FieldOutOfBounds {
  public:
  FieldOutOfBounds(int x, int y) : x(x), y(y) {};
    int x,y;
  };

  class IllegalResolution {};

 private:
  // Private helper functions
  void cinc(int, int, double); // Conditional increase, increase the entry if it exists

  // Private data
  int nsx, nsy;
  Bounds bounds;
  double dx, idx, dy, idy;
  bool wrapX, wrapY;
  double *array;
  double *lap_array;

  double diffusion, lambda;

  std::map<int,double> propReduce; // For doing a proportional reduction (-lambda*n(x))
  double& cue(int, int); // Update the proportional reduce structure
};

#endif
