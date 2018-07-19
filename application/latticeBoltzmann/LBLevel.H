
#ifndef _LBLEVEL_H_
#define _LBLEVEL_H_

/******************************************************************************/
/**
 * \file LBLevel.H
 *
 * \brief Class for controlling Lattice Boltzmann things
 *
 *//*+*************************************************************************/

#include "Parameters.H"
#include "IntVect.H"
#include "LBParameters.H"
#include "DisjointBoxLayout.H"
#include "LevelData.H"
#include "BaseFabMacros.H"

class LBLevel
{
  using LevelSolData = LevelData<BaseFab<Real> >;

/*====================================================================*
 * Public constructors and destructors
 *====================================================================*/

public:

  /// Default Constructor
  LBLevel();
  
  /// Constructor
  LBLevel(const DisjointBoxLayout& a_dbl); // FIXME
  
  /// Copy constructor
  LBLevel(const LBLevel&) = delete;
  
  /// Move constructor
  LBLevel(LBLevel&&) = delete;
  
  /// Assignment constructor
  LBLevel& operator=(const LBLevel&) = delete;
  
  /// Move assignment constructor
  LBLevel& operator=(LBLevel&&) = delete;
  
  /// Destructor
  ~LBLevel() = default;

/*====================================================================*
 * Members functions
 *====================================================================*/

  /// Initialize the data
  void initialData();

  /// Advance all the info
  void advance();

  /// Write to CGNS
  int writePlotFile(int timestep) const;
 
  /// Compute the total mass in the domain
  Real computeTotalMass();

  /// Get current f
  LevelSolData& fi();

  /// Get current fhat
  LevelSolData& fihat();

  /// Set periodic ghost cells
  void setBounceBack(BaseFab<Real>& a_f);


/*====================================================================*
 * Data members
 *====================================================================*/

protected:

  LevelSolData m_f1;            ///< LevelData for timestepping
  LevelSolData m_f2;            ///< LevelData for timestepping
  LevelSolData m_U;             ///< Macroscopic data (density, x-vel, y-vel, z-vel)
  DisjointBoxLayout m_dbl;      ///< DBL all LevelDatas are built on
  Real m_density;               ///< Density
  const Real m_tau = 0.516;     ///< Relaxation time
  bool m_b = true;              ///< True for m_f1 = f, m_f2 = fhat. False switches.
  Copier m_copier;              ///< Copier for data

};

/*******************************************************************************
 *
 * Class LBLevel: inline member definitions
 *
 ******************************************************************************/

/*--------------------------------------------------------------------*/
//  Default constructor
/*--------------------------------------------------------------------*/

inline
LBLevel::LBLevel()
  :
  m_f1(),
  m_f2(),
  m_U(),
  m_dbl(),
  m_density(),
  m_copier()
{
}

/*--------------------------------------------------------------------*/
//  Constructor with DisjointBoxLayout
/*--------------------------------------------------------------------*/

inline
LBLevel::LBLevel(const DisjointBoxLayout& a_dbl)
  :
  m_f1(a_dbl, LBParameters::g_numVelDir, LBParameters::g_numGhost),
  m_f2(a_dbl, LBParameters::g_numVelDir, LBParameters::g_numGhost),
  m_U(a_dbl, 4, LBParameters::g_numGhost), // Need ghostcells?
  m_dbl(a_dbl),
  m_density(1)
{
  initialData();
  m_copier.defineExchangeLD<BaseFab<Real> >(fi(), PeriodicX | PeriodicY, TrimCorner);
  
}

/*--------------------------------------------------------------------*/
//  Initialize the data
/*--------------------------------------------------------------------*/

inline void
LBLevel::initialData()
{
  // Set the initial f
  for (int iVel = 0; iVel !=LBParameters::g_numVelDir; ++iVel)
    {
      m_f1.setVal(iVel, LBParameters::g_weight[iVel] * m_density);
      m_f2.setVal(iVel, LBParameters::g_weight[iVel] * m_density);
      
    }
  
  // Set all the intial macro data
  m_U.setVal(0, m_density);
  m_U.setVal(1, 0);
  m_U.setVal(2, 0);
  m_U.setVal(3, 0);

  

 // This will set each box instead of the entire LevelData
  /*for (DataIterator dit(m_dbl); dit.ok(); ++dit)
    {
      FArrayBox& fabU = m_U[dit]; //BaseFab<Real>
      fabU.setVal(0, m_density*(*dit).localIndex());
      fabU.setVal(1, 0);
      fabU.setVal(2, 0);
      fabU.setVal(3, 0);
      }
  */
}

/*--------------------------------------------------------------------*/
//  Get fi based on m_b 
/*--------------------------------------------------------------------*/

inline LevelData<BaseFab<Real> >&
LBLevel::fi()
{
  return m_b ? m_f1 : m_f2;
}

/*--------------------------------------------------------------------*/
//  Get fihat based on m_b 
/*--------------------------------------------------------------------*/

inline LevelData<BaseFab<Real> >&
LBLevel::fihat()
{
  return m_b ? m_f2 : m_f1;
}


#endif  /* ! defined _LBLEVEL_H_ */
