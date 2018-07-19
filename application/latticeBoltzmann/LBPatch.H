
#ifndef _LBPATCH_H_
#define _LBPATCH_H_

/******************************************************************************/
/**
 * \file LBPatch.H
 *
 * \brief Namespace for doing operations over a BaseFab
 *
 *//*+*************************************************************************/

#include "BaseFabMacros.H"
#include "LBPhysics.H"
#include "Parameters.H"
#include <iostream>

namespace LBPatch
{
  using SolFab = BaseFab<Real>;

/*--------------------------------------------------------------------*/
//  Collision on a Patch
/*--------------------------------------------------------------------*/ 
  void collision(SolFab& a_f, SolFab& a_U, Real tau)
  {
    
    // Call physics for each cell LBPhysics::collision(cell)
    MD_ARRAY_RESTRICT(arrf, a_f);
    MD_ARRAY_RESTRICT(arrU, a_U);
    
    Box center = a_f.box(); // Don't do it on the ghost cells
    center.grow(-1);
    for (int iVel = 0; iVel != LBParameters::g_numVelDir; ++iVel)
      {
        MD_BOXLOOP_OMP(center, i)
          {
            Real density = arrU[MD_IX(i, 0)];
            Real u[3] = {arrU[MD_IX(i, 1)], arrU[MD_IX(i, 2)], arrU[MD_IX(i, 3)]};
            LBPhysics::collision(arrf[MD_IX(i, iVel)], iVel, u, density, tau);
          }
      }
    
  }

/*--------------------------------------------------------------------*/
//  Compute macroscopic properties of density and velocity on a Patch
/*--------------------------------------------------------------------*/
  void macroscopic(SolFab& a_f, SolFab& a_U)
  {
    // LBPhysics::macroscopic at each cell in basefab
    MD_ARRAY_RESTRICT(arrf, a_f);
    MD_ARRAY_RESTRICT(arrU, a_U);

    Real density;
    Real velocity[3];
    Real fi;
    IntVect ei;
    Box center = a_f.box();
    center.grow(-1);

    
    MD_BOXLOOP_OMP(center, i)
      {
        density = 0.;
        memset(velocity, 0, sizeof(velocity)); 
       
        // Compute density and velocity at this position
        for (int iVel = 0; iVel != LBParameters::g_numVelDir; ++iVel)
          {
            ei = LBParameters::latticeVelocity(iVel);
            fi = arrf[MD_IX(i, iVel)];
            density += fi;
            velocity[0] += fi * ei[0];
            velocity[1] += fi * ei[1];
            velocity[2] += fi * ei[2];
          }
        arrU[MD_IX(i, 0)] = density;
        arrU[MD_IX(i, 1)] = velocity[0] / density;
        arrU[MD_IX(i, 2)] = velocity[1] / density;
        arrU[MD_IX(i, 3)] = velocity[2] / density;
        
        
     }
    
  }

/*--------------------------------------------------------------------*/
//  Stream over a Patch. stream fabA into fabB
/*--------------------------------------------------------------------*/
  void stream(SolFab& a_fabA, SolFab& a_fabB)
  {
    
    IntVect ei;
    Box srcBox;
    Box dstBox;
    
    // use basefab copy with properly shrunk and shifted boxes in direction of ei
    for (int iVel = 0; iVel != LBParameters::g_numVelDir; ++iVel)
    {
      ei = LBParameters::latticeVelocity(iVel);
      /*
      srcBox = a_fabA.box();
      srcBox.shift(-ei);
      srcBox &= a_fabA.box();
      dstBox = a_fabB.box();
      dstBox.shift(ei);
      dstBox &= a_fabA.box();
      
      a_fabB.copy(dstBox,
                  iVel,
                  a_fabA,
                  srcBox,
                  iVel,
                  1);
      */
      dstBox = a_fabA.box();
      dstBox.grow(-1);
      srcBox = a_fabA.box();
      srcBox.grow(-1);
      srcBox.shift(-ei);

      a_fabB.copy(dstBox,
                  iVel,
                  a_fabA,
                  srcBox,
                  iVel,
                  1);
      
    }
  }
  
}



#endif /* ! defined _LBPATCH_H_ */
