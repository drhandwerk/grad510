
/******************************************************************************/
/**
 * \file latticeBoltzmann.cpp
 *
 * \brief Lattice-Boltzmann solution to a problem
 *
 *//*+*************************************************************************/

#ifdef USE_MPI
#include "pcgnslib.h"
#else
#include "cgnslib.h"
#endif

#include <omp.h>

#include "DisjointBoxLayout.H"
#include "LBLevel.H"
#include "IntVect.H"
#include "Stopwatch.H"
#include <iostream>
#include <chrono>
// Don't forget to configure with --enable-release
int main(int argc, const char* argv[])
{
  
#pragma omp_set_num_threads(4);
  
#ifdef USE_MPI
  //Initialize MPI
  DisjointBoxLayout::initMPI(argc, argv);
  //int numProc = DisjointBoxLayout::numProc();
  int procID = DisjointBoxLayout::procID();
  const bool masterProc = (procID == 0);
#else
  const bool masterProc = true;
#endif

  Stopwatch<std::chrono::steady_clock> stopwatch;
  // Setup input parameters
  // Setup LBLevel
  Box domain(IntVect::Zero, IntVect(D_DECL(63,31,31)));
  DisjointBoxLayout dbl(domain, 16*IntVect::Unit);
  LBLevel level(dbl);
  int maxTime = 4000;
  //std::cout << "DBL size: " << dbl.size() << std::endl;
  


  if (masterProc) 
    {
      std::cout << "Beginning Lattice-Boltzmann simulation" << std::endl;
      std::cout << "Domain size: " << domain.dimensions() << std::endl;
      std::cout << "Running with " << maxTime << " timesteps." << std::endl;
      std::cout << "Number of local boxes: " << dbl.localSize() << std::endl;
    }

  if (masterProc) {stopwatch.start();}
  for (int t = 0; t < maxTime; ++t)
    {
      //std::cout << "time: " << t << " proc id: " << procID << "\n";
      if (t % 400 == 0) 
        { 
          //Real mass = level.computeTotalMass();
          level.writePlotFile(t);
          if (masterProc)
            {
              std::cout << "Writing to file with t = " << t << std::endl;
              //std::cout << "Total mass: " << mass << std::endl;
            }
        }
      level.advance();
    }
  if (masterProc) {stopwatch.stop();}
      
  level.writePlotFile(maxTime);
  if (masterProc)
    {
      std::cout << "Writing to file with t = " << maxTime << std::endl;
      std::cout << "Time: " <<  stopwatch.time() << std::endl;
    }
      
#ifdef USE_MPI
  DisjointBoxLayout::finalizeMPI();
#endif
  return 0;
}
  
