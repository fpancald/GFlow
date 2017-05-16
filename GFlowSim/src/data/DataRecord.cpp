#include "DataRecord.hpp"

namespace GFlow {
  DataRecord::DataRecord() : writeDirectory("RunData"), delay(1./15.), lastRecord(-delay), recIter(0), nsx(-1), nsy(-1), sdx(-1), sdy(-1), cutoff(-1), skinDepth(-1), recPos(false), recPerf(false) {};

  void DataRecord::startTiming() {
    start_time = high_resolution_clock::now();
  }

  void DataRecord::endTiming() {
    end_time = high_resolution_clock::now();
  }

  void DataRecord::markTime() {
    last_record = high_resolution_clock::now();
  }

  double DataRecord::getElapsedTime() {
    return time_span(end_time, start_time);
  }

  void DataRecord::record(SimData* simData, RealType time) {
    // Return if not enough time has gone by
    if (time-lastRecord<delay) return;

    // Record performance
    if (recPerf) {
      high_resolution_clock::time_point current_time = high_resolution_clock::now();
      performanceRecord.push_back(pair<RealType,RealType>(time,time_span(current_time, last_record)));
      last_record = current_time;
    }

    // Record data
    lastRecord = time;

    // Get the arrays
    RealType *px = simData->getPxPtr();
    RealType *py = simData->getPyPtr();
    RealType *sg = simData->getSgPtr();
    RealType *th = simData->getThPtr();
    int *it = simData->getItPtr();
    int domain_size = simData->getDomainSize();

    // Record data
    if (recPos) {
      vector<PData> positions;
      for (int i=0; i<domain_size; ++i)
	if (it[i]>-1) 
	  positions.push_back(PData(px[i], py[i], sg[i], th[i], it[i], 0));
      // Add to position record
      positionRecord.push_back(positions);
    }

    // Record stat functions
    for (int i=0; i<statFunctions.size(); ++i) {
      StatFunc sf = statFunctions.at(i);
      statFunctionData.at(i).push_back(pair<RealType, RealType>(time, sf(simData)));
    }

    // Increment record counter
    recIter++;
  }

  void DataRecord::getSectorizationData(Sectorization* sectors) {
    // Get width and height of sector grid
    nsx = sectors->getNSX(); 
    nsy = sectors->getNSY();
    // Get sector's width and height
    sdx = sectors->getSDX();
    sdy = sectors->getSDY();
    // Get cutoff and skin depth
    cutoff = sectors->getCutoff();
    skinDepth = sectors->getSkinDepth();
    // Get the number of neighbor lists
    auto verletList = sectors->getVerletList();
    numberOfVerletLists = verletList.size();
    // Get the average neighbors per verlet list
    RealType ave = 0;
    for (const auto& vl : verletList) ave += vl.size();
    ave /= static_cast<RealType>(numberOfVerletLists);
    avePerVerletList = ave;
  }

  void DataRecord::writeData(SimData* simData) {
    // Remove previously existing files
    system(("rm -rf "+writeDirectory).c_str());

    // Create the directory
    mkdir(writeDirectory.c_str(), 0777);

    // Write number
    printToCSV(writeDirectory+"/number.csv", vector<int>(1,recIter));

    // Write walls to file
    if (simData)
      if (!printToCSV(writeDirectory+"/walls.csv", simData->getWalls()))
	cout << "Failed to print walls to [" << writeDirectory << "/walls.csv].\n";

    // Write bounds to file
    if (simData)
      if (!printToCSV(writeDirectory+"/bnds.csv", vector<Bounds>(1,simData->getSimBounds())))
	cout << "Failed to print bounds to [" << writeDirectory << "/bnds.csv].\n";
    
    // Create Positions directory
    mkdir((writeDirectory+"/Pos").c_str(), 0777);
    // Write position data
    for (int i=0; i<positionRecord.size(); ++i)
      if (!printToCSV(writeDirectory+"/Pos/pos", positionRecord.at(i), i))
	cout << "Failed to print to [" << writeDirectory << "/Pos/pos" << i << "].\n";
  }

  void DataRecord::writeRunSummary() {
    std::ofstream fout(writeDirectory+"/run_summary.txt");
    if (fout.fail()) {
      // Write error message
      cout << "Failed to open file [" << writeDirectory << "/run_summary.txt]." << endl;
      return;
    }
    // Print Header
    fout << "**********          SUMMARY          **********\n";
    fout << "**********  GFlow Granular Simulator **********\n";
    fout << "********** 2017, Nathaniel Rupprecht **********\n";
    fout << "***********************************************\n\n";
    // Print timing summary
    RealType elapsedTime = time_span(end_time, start_time);
    fout << "Timing and performance:\n";
    fout << "  - Simulated Time:  " << runTime;
    if (fabs(runTime-actualTime)>0.1) fout << ", actually simulated " << actualTime;
    fout << "\n";
    fout << "  - Actual run Time: " << elapsedTime << "\n";
    fout << "  - Ratio: " << runTime/elapsedTime << "\n";
    fout << "\n";

    // Print the sectorization summary
    fout << "Sectorization summary (as of end of simulation):\n";
    fout << "  - Grid dimensions: " << nsx << " x " << nsy << "\n";
    fout << "  - Grid lengths:    " << sdx << " x " << sdy << "\n";
    fout << "  - Number of verlet lists: " << numberOfVerletLists << "\n";
    fout << "  - Average number per verlet list: " << avePerVerletList << "\n";
    // Close the stream
    fout.close();
  }

  void DataRecord::addStatFunction(StatFunc sf, string name) {
    // Add a place to store this function's data
    statFunctionData.push_back(vector<pair<RealType,RealType> >());

    // Add the stat function
    statFunctions.push_back(sf);

    // Add the stat function's name
    statFunctionName.push_back(name);
  }

  vector<pair<RealType, RealType> > DataRecord::getStatFunctionData(int index) {
    if (index<0 || statFunctionData.size()<=index) throw BadStatFunction(index);
    return statFunctionData.at(index);
  }

  string DataRecord::getStatFunctionName(int index) {
    if (index<0 || statFunctionData.size()<=index) throw BadStatFunction(index);
    return statFunctionName.at(index);
  }

  vector<pair<RealType, RealType> > DataRecord::getPerformanceRecord() {
    return performanceRecord;
  }
  
}