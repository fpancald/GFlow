/// driver.cpp --- Driver file
/// Nathaniel Rupprecht 2016
///

#include "../ArgParse.h"
#include "GFlowBase.h"
//#include "GFlow.h"

using MPI::COMM_WORLD;

#include <iostream>
using std::cout;
using std::endl;

int main(int argc, char** argv) {
  // Simulation parameters
  int number = -1;
  double width = 4;
  double height = 4;
  double radius = 0.05;
  double bR = 0.2;
  double density = 10;
  double velocity = 0.25;
  double dispersion = 0;
  double temperature = 0;
  double time = 1.;
  double start = 0;
  double phi = 0.5;
  int interaction = 0;
  bool interact = true;
  bool seedRand = true;

  // Animation Paramaters
  bool animate = false;
  bool KE      = false;
  bool omega   = false;
  bool cluster = false;
  bool novid   = false;

  // Simulation type
  bool square = true;
  bool buoyancy = false;

  // Initialize MPI
  int rank, numProc;
  MPI::Init();
  rank = MPI::COMM_WORLD.Get_rank();
  numProc = MPI::COMM_WORLD.Get_size();
  MPI::COMM_WORLD.Barrier();

  //----------------------------------------
  // Parse command line arguments
  //----------------------------------------
  ArgParse parser(argc, argv);
  parser.get("number", number);
  parser.get("width", width);
  parser.get("height", height);
  parser.get("radius", radius);
  parser.get("bR", bR);
  parser.get("density", density);
  parser.get("velocity", velocity);
  parser.get("dispersion", dispersion);
  parser.get("temperature", temperature);
  parser.get("time", time);
  parser.get("start", start);
  parser.get("phi", phi);
  parser.get("interaction", interaction);
  parser.get("interact", interact);
  parser.get("srand", seedRand);
  parser.get("animate", animate);
  parser.get("KE", KE);
  parser.get("omega", omega);
  parser.get("cluster", cluster);
  parser.get("novid", novid);
  parser.get("square", square);
  parser.get("buoyancy", buoyancy);
  //----------------------------------------

  // Seed random number generators
  if (seedRand) {
    srand48( std::time(0) );
    srand( std::time(0) );
    seedNormalDistribution();
  }

  // Calculate number of particles given a packing fraction
  if (number==-1) {
    double Vol = width*height;
    number = Vol/(PI*sqr(radius))*phi;
  }

  /// Print condition summary
  if (rank==0) {
    cout << "----------------------- RUN SUMMARY -----------------------\n";
    cout << "Command: ";
    for (int i=0; i<argc; i++) cout << argv[i] << " ";
    cout << endl << endl; // Line break
    cout << "Using " << numProc << " processors.\n";
    cout << "Number: " << number << endl;
    cout << "  ..........................\n";
  }
  MPI::COMM_WORLD.Barrier();

  // Set up the simulation
  GFlowBase simulator;  
  if (buoyancy) simulator.createBuoyancyBox(radius, bR, density, width, height, velocity, dispersion);
  else if (square) simulator.createSquare(number, radius, width, height, velocity, dispersion);
  else throw false; // No selection

  simulator.setTemperature(temperature);
  simulator.setDoInteractions(interact);
  simulator.setStartRec(start);

  simulator.setRecPositions(animate);

  if (KE) simulator.addStatFunction(Stat_KE, "ke");
  if (omega) simulator.addStatFunction(Stat_Omega, "omega");
  if (cluster) simulator.addStatFunction(Stat_Clustering, "cluster");

  if (rank==0) {
    cout << "Dimensions: " << simulator.getWidth() << " x " << simulator.getHeight() << endl;
    cout << "Set up time: " << simulator.getSetUpTime() << endl;
    cout << "  ..........................\n";
  }

  if (interaction!=0) simulator.setInteractionType(interaction);

  // Run the simulation
  simulator.run(time);

  // Print Run summary
  if (rank==0) {
    double runTime = simulator.getRunTime(), transferTime = simulator.getTransferTime();
    int iters = simulator.getIter(), ndx = simulator.getNDX(), ndy = simulator.getNDY();
    cout << "Domains: " << ndx << " x " << ndy << ", Total: " << ndx*ndy << endl;
    cout << "Run Time: " << runTime << ", Sim Time: " << time << endl;
    cout << "Iterations: " << iters << endl;
    cout << "Transfer Time: " << transferTime << " (" << (runTime>0 ? toStr(transferTime/runTime*100) : "---") << "%)" << endl;
    cout << "Ratio: " << (runTime>0 ? toStr(time/runTime) : "---") << "\tTime per iter per particle: " << (iters>0 && number>0 ? mmPreproc(time/iters/number) : "---");
    cout << "\n----------------------- END SUMMARY -----------------------\n\n"; 

    /// Print recorded data
    if (animate) cout << simulator.printAnimationCommand(novid) << endl;
    string stats = simulator.printStatFunctions();
    if (!stats.empty()) cout << stats;
  }
  // End MPI
  MPI::Finalize();
  return 0;
}
