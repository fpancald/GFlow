#include "Simulator.h"

#include <ctime>

int main() {
  // Parameters
  double width = 4.;
  double height = 2.;
  double pA = 0.1;  // Portion that are active
  int trials = 1;   // Number of trials to average over
  double time = 30; // Time to run the simulation for
  int N1 = 0, N2 = 900; // Starting and ending numbers
  int steps = 7;
  int stepSize = (N2-N1)/steps;

  double startRec = 3; // What time to start recording data
  double radius = 0.05;
  double rA = radius;
  double Vol = width*height;
  
  // Seed random number generators
  srand48( std::time(0) );
  srand( std::time(0) );

  //----------------------------------------

  Simulator simulation;
  
  simulation.addStatistic(statPassiveFlow);
  simulation.addStatistic(statActiveFlow);
  simulation.addStatistic(statFlowRatio);

  simulation.setStartRecording(startRec);
  auto start = clock();
  vector<vect<> > dataA, dataP, dataX;
  vect<> bias(0.1, 0);
  int number = N1;
  for (int i=0; i<=steps; i++, number+=stepSize) {
    double act=0, pass=0, xi=0;
    int NP = number*(1-pA), NA = number-NP;
    for (int i=0; i<trials; i++) {
      // Create a new control pipe
      simulation.createControlPipe(NP, NA, radius, 0, bias, rA, width, height);
      // Run the simulation
      simulation.run(time);
      // Record data
      pass += average(simulation.getStatistic(0));
      act += average(simulation.getStatistic(1));
      xi += average(simulation.getStatistic(2));
    }
    act /= (double)trials;
    pass /= (double)trials;
    xi /= (double)trials;
    double phi = number*PI*sqr(radius)/Vol;
    dataA.push_back(vect<>(phi, act));
    dataP.push_back(vect<>(phi, pass));
    dataX.push_back(vect<>(phi, xi));
  }
  auto end = clock();

  return 0;

  cout << "Varies packing ratio\n";
  cout << "Trials: " << trials << ", Trial time: " << time << endl;
  cout << "Percent active: " << pA*100 << "%\n"; 
  cout << "Pipe width, height: " << width << ", " << height << endl;
  cout << "Radius: " << radius << ", Ra = " << rA << endl;
  cout << "Time: " << (double)(end-start)/CLOCKS_PER_SEC << endl;

  // Print list of data
  cout << "act=" << dataA << ";\n";
  cout << "pass=" << dataP << ";\n";
  cout << "xi=" << dataX << ";";

  return 0;
}