#include "StandardSectorization.hpp"

namespace GFlow {

  StandardSectorization::StandardSectorization() : Sectorization(), threshold(0) {};

  StandardSectorization::StandardSectorization(SimData* simData) : Sectorization(simData), threshold(0) {};

  void StandardSectorization::_createVerletLists() {

    // Get position data
    RealType *px = simData->getPxPtr();
    RealType *py = simData->getPyPtr();
    RealType *sg = simData->getSgPtr();
    int *it      = simData->getItPtr();

    // Clear out lists
    verletList.clear();

    // Redo the sectorization so we can make our verlet lists
    _sectorize();

    // Create lists
    for (int y=1; y<nsy-1; ++y)
      for (int x=1; x<nsx-1; ++x)
        for (auto p=sec_at(x,y).begin(); p!=sec_at(x,y).end(); ++p) {
          // Get your data
          int i = *p;
          if (it[i]<0) continue;
          RealType sigma = sg[i];
          VListSubType nlist;
          nlist.push_back(i); // You are at the head of the list

	  // If you are a "small" particle (only need to check adjacent sectors)
	  if (sigma<=threshold) {
	    // Sector you are in
	    auto q = p; ++q; // Check only particles ordered after you
	    for (; q!=sec_at(x,y).end(); ++q) {
	      int j = *q;
	      RealType R = sg[j];
	      if (it[j]<0 || threshold<R) continue;
	      vec2 r = getDisplacement(px[i], py[i], px[j], py[j]);
	      if (sqr(r) < sqr(sigma + R + skinDepth)) nlist.push_back(j);
	    }
	    
	    // Checking lambda
	    auto check = [&] (int sx, int sy) {
	      // Wrapping logic
	      if (wrapX) {
		if (sx==0)     sx = nsx-2;
		if (sx==nsx-1) sx = 1;
	      }
	      if (wrapY) {
		if (sy==0)     sy = nsy-2;
		if (sy==nsy-1) sy = 1;
	      }
	      // Measuring
	      for (const auto &j : sec_at(sx, sy)) {
		if (it[j]<0 || threshold<sg[j]) continue;
		vec2 r = getDisplacement(px[i], py[i], px[j], py[j]);
		if (sqr(r) < sqr(sigma + sg[j] + skinDepth))
		  nlist.push_back(j);
	      }
	    };

	    // Bottom left
	    check(x-1, y-1);
	    // Bottom
	    check(x, y-1);
	    // Left
	    check(x-1, y);
	    // Top left
	    check(x-1, y+1);
	  }

	  else { // Scan more than 1 sector around us
	    // Sector sweep -- works if 2*scanBins# + 1 < ns#
	    RealType scanDist = 2*sigma + skinDepth;
	    int scanBinsX = ceil(scanDist/sdx);
	    int scanBinsY = ceil(scanDist/sdy);
	    int minX = x-scanBinsX, maxX = x+scanBinsX;
	    int minY = y-scanBinsY, maxY = y+scanBinsY;
	    // Adjust for wrapping or bounds
	    if (minX<1) minX = (wrapX ? minX+nsx-2 : 1);
	    if (nsx-2<maxX) maxX = (wrapX ? maxX-nsx+2 : nsx-2);
	    if (minY<1) minY = (wrapY ? minY+nsy-2 : 1);
	    if (nsy-2<maxY) maxY = (wrapY ? maxY-nsy+2 : nsy-2);
	    // Sweep
	    for (int sy=minY; sy!=maxY+1; ++sy) {
	      for (int sx=minX; sx!=maxX+1; ++sx) {
		check(sx,sy,i,it,px,py,sg,sigma,nlist);
		if (wrapX && nsx-1==sx) sx = 0; // The ++ will make this 1
	      }
	      if (wrapY && nsy-1==sy) sy = 0; // The ++ will make this 1
	    }
	  }
	  
          // Add the neighbor list to the collection if you have neighbors
          if (nlist.size()>1) verletList.push_back(nlist);
        }
    // Update position record
    simData->updatePositionRecord();
  }

  void StandardSectorization::_makeSectors() {
    // Find a good cutoff
    auto plist = simData->getParticles();
    if (plist.empty()) {
      RealType minSec = 1.;
      nsx = minSec;
      nsy = minSec;
    }
    else {
      // Find the average volume (/ 2 PI) of a particle
      RealType vol = 0;
      for (const auto& p : plist) vol += sqr(p.sigma);
      vol /= plist.size();
      // Find a radius from that volume
      RealType normalRadius = sqrt(vol), maxR(0);
      for (const auto& p : plist) {
	// Need to use a number slightly bigger than normalRadius to combat floating point rounding
	if (p.sigma<=(1.01)*normalRadius && maxR<p.sigma) maxR = p.sigma;
      }
      cutoff = 2*maxR + skinDepth;
      // First estimate of sdx, sdy
      sdx = sdy = cutoff;
      RealType minSec = 1.;
      nsx = static_cast<int>( max(minSec, (bounds.right-bounds.left)/sdx) );
      nsy = static_cast<int>( max(minSec, (bounds.top-bounds.bottom)/sdy) );
    }
    
    // Actual width and height
    sdx = (bounds.right-bounds.left)/nsx;
    sdy = (bounds.top-bounds.bottom)/nsy;
    isdx = 1./sdx;
    isdy = 1./sdy;

    // Threshold radius - if the radius is smaller than this, we can check every particle within 2*threshold+skinDepth by just checking adjacent sectors
    threshold = 0.5*min(sdy-skinDepth, sdx-skinDepth);

    // Add for edge sectors
    nsx += 2; nsy += 2;

    // Remake sectors
    if (sectors) delete [] sectors;
    sectors = new vector<int>[nsx*nsy+1];
  }

  inline void StandardSectorization::check(int sx, int sy, int i, int* it, RealType* px, RealType *py, RealType *sg, RealType sigma, vector<int>& nlist) {
    for (const auto j : sec_at(sx, sy)) {
      RealType R = sg[j];
      if (it[j]<0 || i==j) continue;
      if (R<sigma || (R==sigma && (px[j]<px[i] || (px[j]==px[i] && py[j]<py[i])))) {
	vec2 r = getDisplacement(px[i], py[i], px[j], py[j]);

	if (sqr(r) < sqr(sigma + R + skinDepth)) {
	  nlist.push_back(j);
	}
      }
    }
  }

}
