/// Header for Simulator.h
/// Nathaniel Rupprecht 5/22/2016

#ifndef SIMULATOR_H
#define SIMULATOR_H

// #include "omp.h"
// #define OMP_ACTIVE

#include "StatFunc.h"
#include "Field.h"
#include "Sectorization.h"
#include "Other/Tensor.h"

#include <functional>
#include <list>
using std::list;

enum BType { WRAP, RANDOM, NONE };
enum PType { PASSIVE, RTSPHERE, SSPHERE, BROWNIAN, BACTERIA, SMARTSPHERE, ERROR }; // ERROR is not a type

// Function that converts string to PType
inline PType getType(string s) {
  if (s=="ABP") return BROWNIAN;
  else if (s=="RTSphere") return RTSPHERE;
  else if (s=="ShearSphere") return SSPHERE;
  else if (s=="SmartSphere") return SMARTSPHERE;
  else if (s=="Passive") return PASSIVE;
  else return ERROR;
}

inline string getString(PType t) {
  if (t==BROWNIAN) return "ABP";
  else if (t==RTSPHERE) return "RTSphere";
  else if (t==SSPHERE) return "ShearSphere";
  else if (t==SMARTSPHERE) return "SmartSphere";
  else if (t==PASSIVE) return "Passive";
  else return "Error";
}

// A function that takes a double (time) and returns a wall position
typedef pair<vect<>, vect<> > WPair;
typedef std::function<WPair(double)> WFunc;

/// The simulator class
class Simulator {
 public:
  Simulator();
  ~Simulator();

  // Initialization
  void createSquare(int, int=0, double=0.025, double=1, double=1);
  void createHopper(int, double=0.025, double=0.14, double=1., double=3., double=0.);
  void createPipe(int, double=0.02, double=1., int=0);
  void createControlPipe(int, int, double=0.02, double=1., double=default_run_force, double rA=-1, double=4., double=2., double=0.);
  void createSedimentationBox(int, double=0.02, double=2., double=2., double=default_run_force, bool=false);
  void createSphereFluid(int, int, double=0.02, double=default_run_force, double=-1, double=10., double=2., double=0., bool=false);
  void createBuoyancyBox(double=0.02, double=0.5, double=5., double=4., double=16., double=0., double=0., double=0., double=0., double=0., bool=false, bool=true);
  void loadBuoyancy(string, double, double, double, bool);
  void createIdealGas(int, double=0.02, double=0.1, double=1., double=1.);
  void createEntropyBox(int, double=0.02);
  void createBacteriaBox(int, double=0.02, double=1., double=1., double=1.);

  // Simulation
  void run(double runLength);
  void bacteriaRun(double runLength);

  // Accessors
  vect<> getShear(vect<>);
  vect<> getFVelocity(vect<>);
  double getEpsilon() { return epsilon; }
  double getDisplayTime() { return dispTime; }
  double getBinXWidth() { return (right-left)/bins; }
  double getBinYWidth() { return (top-bottom)/bins; }
  double getVBinXWidth() { return (maxVx-minVx)/vbins; }
  double getVBinYWidth() { return (maxVy-minVy)/vbins; }
  int getVBinXZero() { return (int)(-vbins*minVx/(maxVx-minVx)); }
  int getVBinYZero() { return (int)(-vbins*minVy/(maxVy-minVy)); }
  double getWidth() { return right - left; }
  double getHeight() { return top - bottom; }
  double getMaxV() { return maxV; }
  double getMaxVx() { return maxVx; }
  double getMinVx() { return minVx; }
  double getMaxVy() { return maxVy; }
  double getMinVy() { return minVy; }
  int getNumber() { return particles.size(); }
  int getIter() { return iter; }
  double getRunTime() { return runTime; }
  double getTime() { return time; }
  bool getDelayTriggeredExit() { return delayTriggeredExit; }
  int getSecX() { return sectorization.getSecX(); }
  int getSecY() { return sectorization.getSecY(); }
  int getTotalSectors() { return sectorization.getTotalSectors(); }
  double getMark(int); // Accesses the value of a mark
  int getMarkSize() { return timeMarks.size(); } // Returns the number of time marks
  double getMarkSlope(); // Gets the ave rate at which marks occur (while marks are occuring)
  double getMarkDiff(); // Gets the difference in time between the first and last marks
  int getSize() { return particles.size(); }
  int getWallSize() { return walls.size(); }
  PType getActiveType() { return activeType; }
  vector<double> getAllRadii();
  vector<vect<> > getAllPositions();
  vector<double> getClusteringRec() { return clusteringRec; }
  vector<vector<double> > getProfile() { return profiles; }
  vector<vect<> > getAveProfile();
  vector<vect<> > getVelProfile();
  vector<vect<> > getFlowXProfile(); // Gets the flow rate as a function of *x*

  /// More bacteria accessors
  double getResourceBenefit() { return alphaR; }
  double getWasteHarm() { return alphaW; }
  double getSecretionCost() { return betaR; }
  double getResourceSaturation() { return csatR; }
  double getWasteSaturation() { return csatW; }
  double getResourceDecay() { return lamR; }
  double getWasteDecay() { return lamW; }
  double getSecretionRate() { return secretionRate; }
  double getEatRate() { return eatRate; }
  double getResourceDiffusion() { return resourceDiffusion; }
  double getWasteDiffusion() { return wasteDiffusion; }
  double getMutationRate() { return mutationRate; }
  double getMutationAmount() { return mutationAmount; }

  // Performance functions
  double avePerSector() { return sectorization.avePerSector(); }
  double aveNeighbors() { return sectorization.aveNeighbors(); }
  double aveMemDiffOfNeighbors() { return sectorization.aveMemDiffOfNeighbors(); }
  int maxMemDiffOfParticles() { return sectorization.maxMemDiffOfParticles(); }

  // Statistic functions
  void addStatistic(statfunc); // Adds a statistic to track
  void addAverage(statfunc); // Add a statistic to find the average of
  int numStatistics() { return statistics.size(); } // Returns the number of statistics we are tracking
  int numAverages() { return averages.size(); }
  vector<vect<> > getStatistic(int i); // Returns a statistic record
  double getAverage(int i);
  int getPSize() { return psize; }
  int getASize() { return asize; }
  vector<double> getDensityXProfile();
  vector<double> getDensityYProfile();
  vector<double> getVelocityXProfile();
  vector<double> getVelocityYProfile();
  double clustering();
  double aveVelocity();
  double aveVelocitySqr();
  double aveKE();
  double highestPosition();
  vect<> netMomentum();
  vect<> netVelocity();
  vector<double> getTimeMarks() { return timeMarks; }
  vector<vect<> > getVelocityDistribution();
  vector<vect<> > getAuxVelocityDistribution();
  Tensor getDistribution();
  Tensor getCollapsedDistribution(int=0); // Average out one of the spatial indices
  Tensor getCollapsedProjectedDistribution(int=0, int=0); // Average out a spatial index and project in a direction. Proj=0 --> Project out x component (get y component)
  Tensor getSpeedDistribution();

  // Mutators
  void noFlow() { setFlowV(0); flowFunc=0; hasDrag=false; }
  void setEpsilon(double e) { epsilon = e; }
  void setAnimationSortChoice(int c) { animationSortChoice = c; }
  void setFlowV(double);
  void setUseVelocityDiff(bool d) { useVelocityDiff = d; }
  void setRecordDist(bool r);
  void setRecordBulk(bool r) { recordBulk = r; }
  void setRecordPressure(bool r) { recordPressure = r; }
  void setDispRate(double r) { dispTime = 1.0/r; }
  void setRecFields(bool r) { recFields = r; }
  void setReplenish(double r) { replenish = r; }
  void setWasteSource(double r) { wasteSource = r; }
  void setEatRate(double e) { eatRate = e; }
  void setSecretionRate(double s) { secretionRate = s; }
  void setDispFactor(double f) { dispFactor = f; }
  void setSectorize(bool s) { sectorize = s; }
  void setSectorDims(int sx, int sy);
  void setDimensions(double, double, double, double);
  void setXLBound(BType b) { xLBound = b; }
  void setXRBound(BType b) { xRBound = b; }
  void setYTBound(BType b) { yTBound = b; }
  void setYBBound(BType b) { yBBound = b; }
  void setYTop(double y) { yTop = y; }
  void setActiveType(PType t) { activeType = t; }
  void setGravity(vect<> g) { gravity = g; }
  void setTemperature(double T) { temperature=T; }
  void setMarkWatch(bool w) { markWatch = w; }
  void setStartRecording(double t) { startRecording = t; }
  void setStopRecording(double t) { stopRecording = t; }
  void setCapturePositions(bool) { capturePositions = true; }
  void setCaptureProfile(bool cp) { captureProfile = cp; } 
  void setCaptureVelProfile(bool cp) { captureVelProfile = cp; }
  void setCaptureVelocity(bool);
  void setStartTime(double t) { startTime = t; }
  void setDelayTime(double t) { delayTime = t; }
  void setMaxIters(int it) { maxIters = it; }
  void setRecAllIters(bool r) { recAllIters = r; }
  void setHasDrag(bool d) { hasDrag = d; }
  void setBins(int p);
  void setVBins(int p);
  void setMaxV(double v) { maxV = v; }
  void setMaxVx(double v) { maxVx = v; }
  void setMinVx(double v) { minVx = v; }
  void setVx(double v) { minVx=maxVx=v; }
  void setMaxVy(double v) { maxVy = v; }
  void setMinVy(double v) { minVy = v; }
  void setVy(double v) { minVy=maxVy=v; }
  void setFlowFunc(std::function<vect<>(vect<>)> f) { flowFunc = f; }
  void removeParticlesAbove(double);
  void discard();
  //// More bacteria mutators
  void setResourceDiffusion(double d) { resourceDiffusion = d; }
  void setWasteDiffusion(double d) { wasteDiffusion = d; }
  void setResourceBenefit(double a) { alphaR = a; }
  void setWasteHarm(double a) { alphaW = a; }
  void setResourceSaturation(double k) { csatR = k; }
  void setWasteSaturation(double k) { csatW = k; }
  void setSecretionCost(double b) { betaR = b; }
  void setResourceDecay(double l) { lamR = l; }
  void setWasteDecay(double l) { lamW = l; }
  void setMutationRate(double mu) { mutationRate = mu; }
  void setMutationAmount(double ds) { mutationAmount = ds; }
  /// Global set functions
  void setParticleDissipation(double);
  void setWallDissipation(double);
  void setParticleCoeff(double);
  void setWallCoeff(double);
  void setParticleDrag(double);

  // Creation Functions
  void addWall(Wall*);
  void addTempWall(Wall*, double);
  void addMovingWall(Wall*, WFunc);
  void addParticle(Particle*);
  void addParticles(int N, double R, double var, double left, double right, double bottom, double top, PType type=PASSIVE, double vmax=-1);
  void addRTSpheres(int N, double R, double var, double left, double right, double bottom, double top, vect<> bias);
  void addActive(vect<>, double, double);
  vector<vect<> > findPackedSolution(int, double, double, double, double, double, double=0.5, double=0.5, vect<> = Zero); // Finds where we can put particles for high packing

  // File functions
  bool loadConfigurationFromFile(string);
  bool createConfigurationFile(string);

  // Display functions
  string printWalls();
  string printAnimationCommand();
  string printSnapshot();
  string printPressureSnapshot();
  string printBulkAnimationCommand();
  string printPressureAnimationCommand();
  string printDPDTAnimationCommand();
  string printResource();
  string printWaste();
  string printFitness();
  
  string printResourceRec();
  string printWasteRec();
  string printFitnessRec();

  string printMaxV();
  string printAveV();
  string printAveVSqr();
  string printKE();
  string printNetMomentum();
  string printNetVelocity();
  string printNetOmega();
  string printAveOmegaSqr();
  string printNetAngularP();
  string printNetTorque();
  
  // print to file function:
  void printBacteriaToFile(); //(string filename); // give filename as argument?
  void printResourceToFile();
  void printWasteToFile();

  // Error Classes
  struct BadDimChoice {};
  struct InvalidLoadFile {
    InvalidLoadFile(string s) : filename(s) {};
    string filename;
  };
  
 private:
  /// Helper functions
  inline void setUpSectorization();
  inline void resetVariables();  // Reset all neccessary variables for the start of a run
  inline void initializeFields(); // Initialize the values of the waste and resource fields
  inline void calculateForces(); // Gravity, flow, particle-particle, and particle-wall forces
  inline void logisticUpdates(); // Time, iteration, and data recording
  inline void objectUpdates();   // Update particles, sectors, and walls
  inline void doResets();        // Reset normal force tracking
  inline void interactions();    // Handle particle and wall interactions
  inline void bacteriaUpdate();  // Handle bacteria
  inline void updateFields();    // Diffusion for fields

  /// Bacteria functions
  inline double getFitness(int, int);

  /// For gradual addition of particles into the system
  bool doGradualAddition; // True if we gradually add particles
  inline void gradualAddition();
  Bounds addSpace;   // In what region we add new particles
  int maxParticles;  // How many particles we want to have in total
  double addDelay;   // How long we wait between adding another particle
  double addTime;    // How long it's been since we added a particle
  double initialV;   // The magnitude of the initial velocity the particles start with

  /// Utility functions  
  inline double maxVelocity(); // Finds the maximum velocity of any particle
  inline double maxAcceleration(); // Finds the maximum acceleration of any particle
  inline vect<> getDisplacement(vect<>, vect<>);
  inline vect<> getDisplacement(Particle*, Particle*);

  inline void update(Particle* &);
  inline void record();
  inline bool inBounds(Particle*);
  inline void setFieldWrapping(bool, bool);
  inline void setFieldDims(int, int);

  /// Data
  double left, right; // Right edge of the simulation
  double bottom, top;   // Top edge of the simulation
  BType xLBound, xRBound, yTBound, yBBound;
  double yTop;    // Where to put the particles back into the simulation (for random insertion)
  vect<> gravity; // Acceleration due to gravity
  std::function<vect<>(vect<>)> flowFunc; // A function describing fluid flow
  bool hasDrag;   // Whether we should apply a drag force to particles
  double flowV;   // Velocity of the flow (if any)
  vect<> pForce;  // A force we use to simulate pressure
  double temperature;   // Temperature of the system
  double viscosity;     // Viscocity, eta, used in brownian diffusion of particles. Water has a viscosity of 1.308e-4 at 10 degrees celcius
  double percent;       // How much of a jamPipe should be obstructed

  /// Buoyancy box
  double wallFrequency; // Frequency of wall vibrations
  double wallAmplitude; // Amplitude of wall vibrations
  std::function<WPair(double)> wallVibration;

  /// Bacteria
  double resourceDiffusion, wasteDiffusion;
  double secretionRate, eatRate;
  double replenish, wasteSource;
  Field resource, waste, buffer;
  bool recFields; // Whether we should record field data or not
  string resourceStr, wasteStr, fitnessStr;
  //// More bacteria parameters
  double alphaR, alphaW, betaR;	// benefit/harm constants
  double csatR, csatW; 		// saturation constants
  double lamR, lamW; 	        // decay constants for fields
  double mutationRate, mutationAmount; //probability and amount of mutation in 'eatRate'

  /// Times etc.
  double time;       // The simulated time
  double epsilon, sqrtEpsilon; // Epsilon (time step) and its square root
  double dispTime;   // Time between recordings (1/dispRate)
  double dispFactor; // Speed up or slow down animation (e.g. 2 -> 2x speed)
  double lastDisp;   // Last time data was recorded
  int iter;          // Simulation iteration
  int recIt;         // Number of times we have recorded data
  int maxIters;      // How many iterations the simulation lasts
  double runTime;    // How long the simulation took to run
  bool running;      // Is the simulation running

  /// Objects
  list<Wall*> walls; // All permanant, stationary walls
  list<pair<Wall*,double> > tempWalls; // All temporary, stationary walls
  vector<pair<Wall*,WFunc> > movingWalls; // All permanant, moving walls
  list<Particle*> particles; // A list containing pointers to all the particles
  PType activeType;   // What type of active particle we should use
  double charRadius; // A characteristic radius, for animating the spheres and creating the main sectorization
  int psize, asize; // Record the number of passive and active particles

  /// Animation
  vector<vector<list<vect<>>>> watchPosCollection; // [ type ] [ recIt ] [ positions ]
  vector<vector<VPair>> bulkCollection; // [ recIt ] [ lines ]
  bool recordBulk;
  vector<vector<Trio>> pressureCollection; // [ recIt ] [ (x,y,P) ]
  vector<vector<Trio>> dpdtCollection;
  bool recordPressure; // To record pressure of dpdt this must be true
  vector<double> charRadiusCollection; // Characteristic radii for particle animation
  vector<string> colorCollection; // What color to use in the animation
  inline string printTable(int);
  vector<vector<WPair> > wallPos; // [ recIt ] [ Wall # ] [ WPair ] For moving walls
  double animationScale; // For ImageSize->{width*animationScale, height*animationScale}
  // Animation sorting (what type of animation to use)
  int animationSortChoice; // 0 - Active/Passive, 2 - Large/Small, 2 - Large only, DEFAULT - Everything together
  double radiusDivide; // The cutoff for large vs. small

  /// Statistics
  vector<statfunc> statistics;
  vector<statfunc> averages;
  vector<vector<vect<> > > statRec; // the vect is for {t, f(t)}
  vector<double> averageRec; // Record values
  vector<double> clusteringRec; // Record clustering
  void resetStatistics();
  bool recAllIters;
  
  /// Velocity analysis
  vector<double> velocityDistribution;
  vector<double> auxVelocityDistribution;
  double maxV; // Maximum velocity magnitude to bin
  double maxF; // Maximum proportional velocity to bin
  int vbins;

  /// Total distribution P[position][velocity]
  bool recordDist;
  Tensor distribution; // The total distribution of positions and velocities (averaged over time)
  double maxVx, minVx, maxVy, minVy; // Maximum/Minimum velocities to bin for full distribution
  vect<> binVelocity(int, int);
  bool useVelocityDiff;

  /// Marks and recording
  vector<double> timeMarks;
  double lastMark;   // The last time a time mark was recorded
  bool markWatch;    // Whether we should break the simulation based on marks
  double startRecording; // When to start recording position (and other) data
  double stopRecording; // When to stop recording position (and other) data
  double startTime;  // When to start looking for time marks
  double delayTime;  // How long between marks counts as a jam
  bool delayTriggeredExit; // If a long enough delay between marks causes the simulation to stop running
  bool recordClustering; // Whether we should record clustering data
  int bins;                         // Binning for space
  vector<vector<double> > profiles; // For density y-profile //**
  vector<double> aveProfile;
  vector<double> velProfile;
  vector<double> flowXProfile;
  inline void updateProfile();     // Update density profile (radial slice)
  inline void updateVelProfile();  // Update velocity profile (radial slice)
  inline void updateFlowXProfile();

  /// Sectorization
  Sectorization sectorization; // The sectorization for the particles
  int secX, secY; // Width and height of sector grid
  bool sectorize; // Whether to use sector based interactions
  vector<sectorFunction> sFunctionList; // List of sector functions (e.g. pressure simulating functions)
  bool doInteractions; // Do particle-particle interactions

  /// What data to save
  bool capturePositions; // Whether we should record the positions of particles and walls
  bool captureProfile;     // Whether we should record profile data
  bool captureVelProfile;  // Whether we should record velocity profile data
  bool captureLongProfile; // Whether we should capture profile vs time data
  bool captureVelocity;  // Whether we should record velocity distribution data

  // Special
  bool needSee;         // True if some particles need to "see" the simulation
};

#endif
