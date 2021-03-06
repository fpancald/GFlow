/* 
 * Author: Nathaniel Rupprecht
 * Start Data: May 11, 2017
 *
 */

#include "../creation/Creator.hpp"
#include "../creation/FileParser.hpp"
#include "../integrators/VelocityVerletIntegrator.hpp"

// Include simulation types
#include "../simulation/StandardSimulation.hpp"
#include "../simulation/FractureSimulation.hpp"

using namespace GFlow;

int main (int argc, char** argv) {
  // Set up MPI
#if USE_MPI == 1
#if _CLANG_ == 1
  int rank, numProc;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &numProc);
#else
  MPI::Init(argc, argv);
  int rank = MPI::COMM_WORLD.Get_rank();
  int numProc = MPI::COMM_WORLD.Get_size();
#endif
#endif

  // Simulation creation options
  bool fracture = false;
  bool print = false;
  // Read some simulation options
  ArgParse parser(argc, argv);
  parser.get("fracture", fracture);
  parser.get("print", print);

  // Create a simulation
  SimulationBase *simulation = nullptr;
  if (fracture) simulation = new FractureSimulation;
  else simulation = new StandardSimulation;

  // Set up the simulation
  simulation->setUp(argc, argv);

  // Update checked flags
  simulation->updateFlagCheck("fracture");

  // Parse command line arguments
  simulation->parse();

  // Run the simulation
  simulation->run();

  // Write data from the simulation
  simulation->write();

  if (fracture && print) {
    FractureSimulation* sim = dynamic_cast<FractureSimulation*>(simulation);
    cout << "breaks=" << sim->getCumNumBreaks() << ";\n";
    cout << "bonds="  << sim->getCumNumBonds()  << ";\n";
    ofstream fout("dataDump.txt");
    if (fout.fail()) cout << "Data dump failure.\n";
    else {
      auto breakages = sim->getBreakages();
      for (auto &dat : breakages) {
	fout << "t=" << std::get<0>(dat) << ": " << std::get<1>(dat) << ", " << std::get<2>(dat) << "\n";
      }
      if (breakages.empty()) fout << "None.\n";
      fout.close();
    }
  }

  // Finalize mpi
#if USE_MPI == 1
#if _CLANG_ == 1
  MPI_Finalize();
#else
  MPI::Finalize();
#endif
#endif

  // The program is done. Return.
  return 0;
}
