/******************************************************************************/
/**
 * \file LBLevel.cpp
 *
 * \brief Specializations for classes in LBLevel.H
 *
 *//*+*************************************************************************/

#ifdef USE_MPI
#include "pcgnslib.h"
#else
#include "cgnslib.h"
#endif

#include "LBLevel.H"
#include "LBPatch.H"

/*--------------------------------------------------------------------*/
//  Advance for time step
/** To advance over LBLevel, update each Patch
 *//*-----------------------------------------------------------------*/

void LBLevel::advance()
{
  // Collision
  for (DataIterator dit(m_dbl); dit.ok(); ++dit)
    {
      FArrayBox& f = fi()[dit];
      FArrayBox& U = m_U[dit];
      
      LBPatch::collision(f, U, m_tau);
    }

  // Set intiterior and periodic ghost cells
  fi().exchange(m_copier);
  
  // Set bounce back, stream, compute macroscopic
  for (DataIterator dit(m_dbl); dit.ok(); ++dit)
    {
      FArrayBox& f = fi()[dit];
      FArrayBox& fhat = fihat()[dit];
      FArrayBox& U = m_U[dit];
      
      setBounceBack(f);
      LBPatch::stream(f, fhat); // stream f into fhat
      LBPatch::macroscopic(fhat, U);
    }

     m_b = !m_b; // swap so f1 is newest after stream

 
}
  



/*--------------------------------------------------------------------*/
//  Plot to CGNS
/** Plot the current data
 *  \return             0 if success
 *//*-----------------------------------------------------------------*/

int LBLevel::writePlotFile(int timestep) const
{
  const char *const variableNames[] = // Could do LBParameters.stateNames()
    {
      "Density",
      "VelocityX",
      "VelocityY",
      "VelocityZ"
    };
  
  // Let the origin of the coordinate system be 0
  IntVect origin = IntVect::Zero;
  // Let the mesh spacing be 1
  Real dx = 1.;

  //--Write the data

  int cgerr;

  // Open the CGNS file
  int indexFile;
  char fileName[33];// = "solution000.cgns";
  sprintf(fileName, "./plot/Solution_%05d.cgns", timestep);
#ifdef USE_MPI
  cgerr = cgp_open(fileName, CG_MODE_WRITE, &indexFile);
#else
  cgerr = cg_open(fileName, CG_MODE_WRITE, &indexFile);
#endif

  // Create the base node
  int indexBase;
  int iCellDim = g_SpaceDim;
  int iPhysDim = g_SpaceDim;
  cgerr = cg_base_write(indexFile, "Base", iCellDim, iPhysDim, &indexBase);

  // Write the zones and coordinates (indexZoneOffset needs to be determined
  // so we can use 'indexCGNSzone = globalBoxIndex + indexZoneOffset'.  This
  // value is almost certainly 1).

  int indexZoneOffset;  // The difference between CGNS indexZone and the
                        // globalBoxIndex.
  cgerr = m_dbl.writeCGNSZoneGrid(indexFile,
                                  indexBase,
                                  indexZoneOffset,
                                  origin,
                                  dx);

  // Write the solution data
  cgerr = m_U.writeCGNSSolData(indexFile,
                             indexBase,
                             indexZoneOffset,
                             variableNames);

  // Close the CGNS file
#ifdef USE_MPI
  cgerr = cgp_close(indexFile);
#else
  cgerr = cg_close(indexFile);
#endif

  return cgerr;
  
}



/*--------------------------------------------------------------------*/
//  Set the bounce back condition on the wall
/*--------------------------------------------------------------------*/

void LBLevel::setBounceBack(BaseFab<Real>& a_f)
{
  
  // Check if on top or bottom
  Box fullDomain = m_dbl.problemDomain();
  fullDomain.grow(1); // Need ghost cells
  Box test = a_f.box();
  test.shift(IntVect(0,0,-1)); // shift down by one. If still in domain, then it's on the top. 

  if (fullDomain.contains(test) ) // top
    {
      // Get box for top layer of interior cells
      Box topCellsBox = a_f.box();     // 18x18x18 since has ghost cells
      topCellsBox.grow(-1);            // 16x16x16 interior cells
      topCellsBox.growLo(-15 , 2);     // 16x16x1 layer of interior cells
      
      Box ghostCellsBox;
      IntVect ei;
      std::vector<int> v = {6, 13, 14, 17, 18}; // Velocity directions with +z
      for (auto iVel = v.begin(); iVel != v.end(); ++iVel) 
        {
          ei = LBParameters::latticeVelocity(*iVel);
          // Set bounce back for each ei. Just use a fab copy. 
          // srcBox is interior cells(topCellsBox). dstBox is same box shifted by ei
          // source comp is iVel. dstComp is opposite of iVel.
          ghostCellsBox = topCellsBox;
          ghostCellsBox.shift(ei);
          
          a_f.copy(ghostCellsBox,
                   LBParameters::oppositeVelDir(*iVel), 
                   a_f,
                   topCellsBox,
                   *iVel,
                   1);
          
        }
    }
  else //bottom
    {
      Box bottomCellsBox = a_f.box();   // 18x18x18 since has ghost cells
      bottomCellsBox.grow(-1);          // 16x16x16 interior cells
      bottomCellsBox.growHi(-15, 2);    // 16x16x1 layer of interior cells
      
      Box ghostCellsBox;
      IntVect ei;
      
      std::vector<int> v = {5, 11, 12, 15, 16}; // Velocity directions with -z
      for (auto iVel = v.begin(); iVel != v.end(); ++iVel) 
        {
          ei = LBParameters::latticeVelocity(*iVel);
          ghostCellsBox = bottomCellsBox; 
          ghostCellsBox.shift(ei);
          
          a_f.copy(ghostCellsBox,
                   LBParameters::oppositeVelDir(*iVel),
                   a_f,
                   bottomCellsBox,
                   *iVel,
                   1);
        }
    }
  

 
  
  
}


/*--------------------------------------------------------------------*/
//  Compute mass in domain
/** Just a sum of fi()
 *  \return             Total mass in domain in process 0
 *//*-----------------------------------------------------------------*/

Real LBLevel::computeTotalMass()
{
  Real localDomainMass = 0.; 
  for (DataIterator dit(m_dbl); dit.ok(); ++dit)  //**FIX m_boxes
    {
      const Box& box = m_dbl[dit];
      MD_ARRAY_RESTRICT(arrfi, fi()[dit]);  //**FIX method to access current fi
      for (int iVel = 0; iVel != LBParameters::g_numVelDir; ++iVel)
        {
          MD_BOXLOOP_OMP(box, i)
            {
              localDomainMass += arrfi[MD_IX(i, iVel)];
            }
        } 
     } 
  Real globalDomainMass = localDomainMass; 
#ifdef USE_MPI
  // Accumulated the result into process 0
  MPI_Reduce(&localDomainMass, &globalDomainMass, 1, BXFR_MPI_REAL, MPI_SUM, 0,
             MPI_COMM_WORLD);
#endif
                            
return globalDomainMass;


}
