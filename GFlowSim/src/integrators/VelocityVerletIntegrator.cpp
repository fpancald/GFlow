#include "VelocityVerletIntegrator.hpp"

namespace GFlow {
  VelocityVerletIntegrator::VelocityVerletIntegrator() : Integrator(), updateDelay(0.002), updateTimer(0) {};

  VelocityVerletIntegrator::VelocityVerletIntegrator(SimData* sim) : Integrator(sim), updateDelay(0.002), updateTimer(0) {};

  VelocityVerletIntegrator::VelocityVerletIntegrator(SimData* sim, DataRecord* dat) : Integrator(sim, dat) {};

  void VelocityVerletIntegrator::addDragForce(DragForce* drag) {
    dragForces.push_back(drag);
  }
  
  void VelocityVerletIntegrator::_integrate() {
    // Make sure we have a simulation to integrate
    if (simData==nullptr || sectors==nullptr) return;
    // Reset iter
    iter = 0;
    // Make sure we have initial verlet lists
    sectors->createVerletLists(true);
    sectors->createWallLists(true);
    // Set initial record time
    if (dataRecord) dataRecord->markTime();
    // Do the integration loop
    while (iter<maxIter) {
      preStep();
      integrateStep();
      postStep();
    }
    // Get final sectors data
    if (dataRecord) dataRecord->getSectorizationData(sectors);
  }
  
  void VelocityVerletIntegrator::preStep() {
    // Nothing
  }
  
  void VelocityVerletIntegrator::integrateStep() {
    // First VV half kick
    firstHalfKick();
    
    // Update sectors
    if (updateDelay<updateTimer) {
      // Reset timer
      updateTimer = 0;
      // If using mpi
#ifdef USE_MPI
      // Exchange data with other processors
      simData->atomMove();
      simData->atomCopy();
#endif
      // Update sectorization
      sectors->createVerletLists();
      sectors->createWallLists(true);
    }

    // Calculate forces
    const auto& verletList = sectors->getVerletList();
    const auto& wallList   = sectors->getWallList();
    forceHandler->pForces(verletList, simData);
    forceHandler->wForces(wallList, simData);

    // Second VV half kick
    secondHalfKick();
  }
  
  void VelocityVerletIntegrator::postStep() {
    // Update iteration
    ++iter;
    // Update times
    time += dt;
    updateTimer += dt;
    // Update data recorder
    if (dataRecord) dataRecord->record(simData, time);
  }

  void VelocityVerletIntegrator::firstHalfKick() {
    // Get data
    RealType *px = simData->getPxPtr();
    RealType *py = simData->getPyPtr();
    RealType *vx = simData->getVxPtr();
    RealType *vy = simData->getVyPtr();
    RealType *fx = simData->getFxPtr();
    RealType *fy = simData->getFyPtr();
    RealType *th = simData->getThPtr();
    RealType *om = simData->getOmPtr();
    RealType *tq = simData->getTqPtr();
    RealType *im = simData->getImPtr();
    RealType *iI = simData->getIiPtr();

    // Get the number of particles we need to update
    int domain_size = simData->getDomainSize();
    double hdt = 0.5*dt;
#pragma vector aligned
#pragma simd
    for (int i=0; i<domain_size; ++i) {
      // Update linear variables
      vx[i] += hdt*im[i]*fx[i];
      vy[i] += hdt*im[i]*fy[i];
      px[i] += dt*vx[i];
      py[i] += dt*vy[i];
      // Wrap position
      simData->wrap(px[i], py[i]);
      // Zero force
      fx[i] = 0;
      fy[i] = 0;
      // Update angular variables
      om[i] += hdt*iI[i]*tq[i];
      th[i] += dt*om[i];
      // Wrap theta
      simData->wrap(th[i]);
      // Zero torque
      tq[i] = 0;
    }
    
    // Apply drag forces
    for (const auto& force : dragForces) {
      for (int i=0; i<domain_size; ++i)
	force->applyForce(simData, i);
    }
  }
  
  void VelocityVerletIntegrator::secondHalfKick() {
    // Get the neccessary data
    RealType *px = simData->getPxPtr();
    RealType *py = simData->getPyPtr();
    RealType *vx = simData->getVxPtr();
    RealType *vy = simData->getVyPtr();
    RealType *fx = simData->getFxPtr();
    RealType *fy = simData->getFyPtr();
    RealType *th = simData->getThPtr();
    RealType *om = simData->getOmPtr();
    RealType *tq = simData->getTqPtr();
    RealType *im = simData->getImPtr();
    RealType *iI = simData->getIiPtr();
    
    // Get the number of particles we need to update
    int domain_size = simData->getDomainSize();
    // Do second half-kick
    RealType hdt = 0.5*dt;
#pragma vector aligned
#pragma simd
    for (int i=0; i<domain_size; ++i) {
      vx[i] += hdt*im[i]*fx[i];
      vy[i] += hdt*im[i]*fy[i];
      om[i] += hdt*iI[i]*tq[i];
    }
  }
  
}