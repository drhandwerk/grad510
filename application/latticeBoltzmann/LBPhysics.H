
#ifndef _LBPHYSICS_H_
#define _LBPHYSICS_H_

/******************************************************************************/
/**
 * \file LBPhysics.H
 *
 * \brief Namespace for doing operations with an individual cell
 *
 *//*+*************************************************************************/

#include "IntVect.H"

namespace LBPhysics
{
  Real G[3] = {1.042e-6, 0, 0}; // Body Force

  /// BGK Collision
  void collision(Real& fi, int& iVel, Real* u, Real& density, Real& tau)
  {
    IntVect ei = LBParameters::latticeVelocity(iVel);
    Real wi = LBParameters::g_weight[iVel];
    
    // Compute feq
    Real eDotU = ei[0]*u[0] + ei[1]*u[1] + ei[2]*u[2];
    Real uu = u[0]*u[0] + u[1]*u[1] + u[2]*u[2];
    Real feq = wi * density * (1 + 3*eDotU + 9*eDotU*eDotU/2 - 3*uu/2);
    
    fi += ((feq - fi) / tau) + 3 * wi * (ei[0]*G[0] + ei[1]*G[1] + ei[2]*G[2]);

  }
  
  /// Compute the macroscopic properties of velocity sum fi * ei
  IntVect macroscopic(Real fi)
  {
    return IntVect::Zero;
    
  }
  
}

#endif /* ! defined _LBPATCH_H_ */
