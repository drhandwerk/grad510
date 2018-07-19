
/******************************************************************************/
/**
 * \file DisjointBoxLayout.cpp
 *
 * \brief Non-inline definitions for classes in DisjointBoxLayout.H
 *
 *//*+*************************************************************************/

#include <cstdio>

#ifdef USE_MPI
#include <mpi.h>
#endif

#ifndef NO_CGNS
#ifdef USE_MPI
#include "pcgnslib.h"
#else
#include "cgnslib.h"
#endif
#endif

#include "DisjointBoxLayout.H"
#include "LayoutIterator.H"
#include "BaseFab.H"
#include "BaseFabMacros.H"
#include <iostream>


/*******************************************************************************
 *
 * Class DisjointBoxLayout: static member initialization
 *
 ******************************************************************************/

int DisjointBoxLayout::s_numProc = 1;
int DisjointBoxLayout::s_procID = 0;


/*******************************************************************************
 *
 * Class DisjointBoxLayout: member definitions
 *
 ******************************************************************************/

/*--------------------------------------------------------------------*/
//  Default constructor
/*--------------------------------------------------------------------*/

DisjointBoxLayout::DisjointBoxLayout()
  :
  m_stride(IntVect::Zero),
  m_numBox(IntVect::Zero),
  m_size(0),
  m_boxes(),
  m_localIdxBeg(0),
  m_numLocalBox(0)
{
}

/*--------------------------------------------------------------------*/
//  Constructor
/** \param[in] a_domain The problem domain
 *  \param[in] a_maxBoxSize
 *                      Maximum box size in each direction
 *  The problem domain is partitioned into boxes, each having maximum
 *  size in a dimension given by a_maxBoxSize.  Depending on how well
 *  the boxes fit into the domain, either a few boxes have a size
 *  reduced by 1 or there is one box at the end with a significantly
 *  different size.  Leading boxes in each direction usually have
 *  a_maxBoxSize dimensions.
 *//*-----------------------------------------------------------------*/

DisjointBoxLayout::DisjointBoxLayout(const Box&     a_domain,
                                     const IntVect& a_maxBoxSize)
{
  define(a_domain, a_maxBoxSize);
}

/*--------------------------------------------------------------------*/
//  Define (weak construction)
/** \param[in] a_domain The problem domain
 *  \param[in] a_maxBoxSize
 *                      Maximum box size in each direction
 *  The problem domain is partitioned into boxes, each having maximum
 *  size in a dimension given by a_maxBoxSize.  Must fit evenly.  
 *  Leading boxes in each direction usually have
 *  a_maxBoxSize dimensions.
 *//*-----------------------------------------------------------------*/

void
DisjointBoxLayout::define(const Box& a_domain, const IntVect& a_maxBoxSize)
{
  m_domain = a_domain;
  const IntVect domainSize =
    a_domain.hiVect() - a_domain.loVect() + IntVect::Unit;

//--Find the number of boxes in each direction.  They should fit evenly into
//--the domain.

  // The number of boxes in each direction
  m_numBox = domainSize/a_maxBoxSize;
  // Make sure the boxes fit evenly into the domain
  CH_assert(m_numBox*a_maxBoxSize == domainSize);

//--Define the conceptual array of boxes

  // We store a 1-D array of boxes.  These are the strides to access a
  // neighbour box in each direction.  (Note: Fortran ordering)
  D_TERM(m_stride[0] = 1;,
         m_stride[1] = m_stride[0]*m_numBox[0];,
         m_stride[2] = m_stride[1]*m_numBox[1];)
    m_size = m_stride[g_SpaceDim-1]*m_numBox[g_SpaceDim-1];

  // Allocate the array of boxes
  m_boxes = std::make_shared<std::vector<BoxEntry>>(m_size);

  // Number of boxes per processor
  const int boxPerProc = m_size/numProc();
  m_numLocalBox = boxPerProc;
  m_localIdxBeg = procID()*boxPerProc;
  // Make sure the boxes fit evenly into the processors
  CH_assert(m_size == boxPerProc*numProc());
  
//--Define the individual boxes and processor assignments for 'm_boxes'

//**FIXME
  Box currBox = Box(a_domain.loVect(), a_domain.loVect() + a_maxBoxSize - IntVect::Unit);
  int linIdxBox = 0; // over all processes
  
  for (int procIdx = 0; procIdx != numProc(); ++procIdx)
  {
		//for (int boxIdx = 0; boxIdx != m_numBox.product(); ++boxIdx)
    for (int boxIdx = 0; boxIdx != boxPerProc; ++boxIdx, ++linIdxBox)  
		{
			m_boxes->operator[](linIdxBox).proc = procIdx;
			(m_boxes->operator[](linIdxBox).box).define(currBox.loVect(), currBox.hiVect());

			currBox.shift(a_maxBoxSize[0],0); // Shift in x
			if (!a_domain.contains(currBox)) // If x-shift put us outside. Reset x. Shift y.
			{
				currBox.shift(a_maxBoxSize[1],1);
				currBox.shift(-a_maxBoxSize[0]*m_numBox[0],0);
			}
			if (!a_domain.contains(currBox))
			{
				currBox.shift(a_maxBoxSize[2],2);
				currBox.shift(-a_maxBoxSize[1]*m_numBox[1],1);
			}
		}
  }
  
}

/*--------------------------------------------------------------------*/
//  Define with deep copy
/** This routine performs a deep copy, making a completely separate
 *  array of boxes
 *  \param[in] a_dbl    DBL to copy
 *//*-----------------------------------------------------------------*/

void
DisjointBoxLayout::defineDeepCopy(const DisjointBoxLayout& a_dbl)
{
  m_stride = a_dbl.m_stride;
  m_numBox = a_dbl.m_numBox;
  m_size = a_dbl.m_size;
  m_boxes = std::make_shared<std::vector<BoxEntry> >(m_size);
  for (int i = 0; i != m_size; ++i)
    {
      getLinear(i) = a_dbl.getLinear(i);
    }
}

#ifndef NO_CGNS
/*--------------------------------------------------------------------*/
//  Write CGNS zone and grid to a file
/** The CGNS file must be open
 *  \param[in]  a_indexFile
 *                      CGNS index of file
 *  \param[in]  a_indexBase
 *                      CGNS index of base node
 *  \param[out] a_indexZoneOffset
 *                      The difference between CGNS indexZone and the
 *                      globalBoxIndex
 *  \param[in]  a_origin
 *                      Origin of the problem domain (assumed to be
 *                      lower vertex)
 *  \param[in]  a_dx    Physical mesh spacing in each direction
 *  \return             0  Success
 *                      >0 1+ the global index of the box that failed
 *  \note
 *  <ul>
 *    <li> Zones are numbered based on their global index (+1) in the
 *         layout
 *  </ul>
 *//*-----------------------------------------------------------------*/

#ifdef USE_MPI
// Local storage of indices for a Box
namespace{
struct CGNSIndices
{
  int indexCoord[3];
};
}
#endif

int
DisjointBoxLayout::writeCGNSZoneGrid(const int      a_indexFile,
                                     const int      a_indexBase,
                                     int&           a_indexZoneOffset,
                                     const IntVect& a_origin,
                                     const Real     a_dx) const
{
  a_indexZoneOffset = -1;
  int cgerr;
  char zoneName[33];
  cgsize_t isize[3][g_SpaceDim];
  // Boundary vertex information (always zero for structured grids)
  D_EXPR(isize[2][0] = 0,
         isize[2][1] = 0,
         isize[2][2] = 0);
  
#ifdef USE_MPI
  std::vector<CGNSIndices> localCGNSIndices(localSize());
#endif

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *
 * Bulk writing of the zone meta-data -- all processors write the same
 * information and note use of LayoutIterator
 *- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  for (LayoutIterator lit(*this); lit.ok(); ++lit)
    {
      const int globalBoxIndex = (*lit).globalIndex();
#ifdef USE_MPI
      const int localBoxIndex  = (*lit).localIndex();
#endif
      D_EXPR(isize[0][0] = (*this)[lit].dimensions()[0] + 1,
             isize[0][1] = (*this)[lit].dimensions()[1] + 1,
             isize[0][2] = (*this)[lit].dimensions()[2] + 1);
      D_EXPR(isize[1][0] = (*this)[lit].dimensions()[0],
             isize[1][1] = (*this)[lit].dimensions()[1],
             isize[1][2] = (*this)[lit].dimensions()[2]);
      //**FIXME       
      
      int indexZone;
      sprintf(zoneName, "Box_%06d", globalBoxIndex);
      cgerr = cg_zone_write(a_indexFile, a_indexBase, zoneName, *isize, Structured, &indexZone);
      if (a_indexZoneOffset < 0)
        {
          a_indexZoneOffset = indexZone - globalBoxIndex;
        }
      CH_assert(indexZone == (globalBoxIndex + a_indexZoneOffset));
      //std::cout << "cgerr: " << cgerr << std::endl;
      //std::cout << "m_boxes local size: " << localSize() << std::endl;
      //std::cout << "localIdxBegin: " << localIdxBegin() << " localIdxEnd: " << localIdxEnd() <<  " on proc " << procID() << std::endl;
      //if (cgerr) {std::cout << "Error: on proc " << procID() <<std::endl; cg_error_print();}
      //std::cout << "ProcID(): " << procID() << " proc(lit): " << proc(lit) << " localidx: " << localBoxIndex << "  globalidx: " << globalBoxIndex << std::endl;
      if (cgerr) return globalBoxIndex+1;
      // Write the coordinate meta-data (if parallel)
#ifdef USE_MPI
      int indexCoord[3];
      
      D_TERM(
        cgp_coord_write(a_indexFile, a_indexBase, indexZone,
                        CGNS_REAL, "CoordinateX", &indexCoord[0]);,
        cgp_coord_write(a_indexFile, a_indexBase, indexZone,
                        CGNS_REAL, "CoordinateY", &indexCoord[1]);,
        cgp_coord_write(a_indexFile, a_indexBase, indexZone,
                        CGNS_REAL, "CoordinateZ", &indexCoord[2]);)
      if (procID() == proc(lit))
        {
          CGNSIndices& thisCGNSIndices = localCGNSIndices[localBoxIndex];
          for (int dir = 0; dir != g_SpaceDim; ++dir)
            {
              thisCGNSIndices.indexCoord[dir] = indexCoord[dir];
            }
        }
#endif
    }
  
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *
 * Collective writing of the coordinate data -- each processor writes
 * its own information (note use of DataIterator)
 *- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

//--These variables describe the shape of the array in the CGNS file.  The min
//--corner of the core grid has index 1.  They are only used for parallel
//--writes.

#ifdef USE_MPI
  cgsize_t rmin[g_SpaceDim];
  cgsize_t rmax[g_SpaceDim];
#endif

  for (DataIterator dit(*this); dit.ok(); ++dit)
    {
      const int globalBoxIndex = (*dit).globalIndex();
      const int indexZone = globalBoxIndex + a_indexZoneOffset;
      Box box = this->operator[](dit);
      box.growHi(1);  // Since we need vertices
      const IntVect loV = box.loVect();
      const IntVect hiV = box.hiVect();
      BaseFab<Real> coords(box, 1);
#ifdef USE_MPI
      const int localBoxIndex = (*dit).localIndex();
      CGNSIndices& thisCGNSIndices = localCGNSIndices[localBoxIndex];
      for (int dir = 0; dir != g_SpaceDim; ++dir)
        {
          rmin[dir] = 1;
          rmax[dir] = box.dimensions()[dir];
        }
#else
      int indexCoord;
#endif
      
      // X
      {
        //**FIXME
        MD_ARRAY_RESTRICT(arrCoord, coords);
        MD_BOXLOOP(box, i)
          {
            arrCoord[MD_IX(i,0)] = i0;
          }
#ifdef USE_MPI
        cgerr = cgp_coord_write_data(a_indexFile, a_indexBase, indexZone, thisCGNSIndices.indexCoord[0], rmin, rmax, coords.dataPtr());
#else
        cgerr = cg_coord_write(a_indexFile, a_indexBase, indexZone, CGNS_REAL, "CoordinateX", coords.dataPtr(), &indexCoord);
#endif
        if (cgerr) return globalBoxIndex+1;
      }

      // Y
      if (g_SpaceDim >= 2)
        {
          //**FIXME
          MD_ARRAY_RESTRICT(arrCoord, coords);
          MD_BOXLOOP(box, i)
            {
              arrCoord[MD_IX(i,0)] = i1;
            }
#ifdef USE_MPI
          cgerr = cgp_coord_write_data(a_indexFile, a_indexBase, indexZone, thisCGNSIndices.indexCoord[1], rmin, rmax, coords.dataPtr());
#else
          cgerr = cg_coord_write(a_indexFile, a_indexBase, indexZone, CGNS_REAL, "CoordinateY", coords.dataPtr(), &indexCoord);
#endif
          if (cgerr) return globalBoxIndex+1;
        }

      // Z
      if (g_SpaceDim >= 3)
        {
          MD_ARRAY_RESTRICT(arrCoord, coords);
          MD_BOXLOOP(box, i)
            {
              arrCoord[MD_IX(i,0)] = i2;
            }
#ifdef USE_MPI
          cgerr = cgp_coord_write_data(a_indexFile, a_indexBase, indexZone, thisCGNSIndices.indexCoord[2], rmin, rmax, coords.dataPtr());
#else
          cgerr = cg_coord_write(a_indexFile, a_indexBase, indexZone, CGNS_REAL, "CoordinateZ", coords.dataPtr(), &indexCoord);
#endif
          
          if (cgerr) return globalBoxIndex+1;
        }
    }
  return 0;
}
#endif  /* CGNS */

/*--------------------------------------------------------------------*/
//  Initialize MPI
/** Any application or test using MPI must call this routine first
 *//*-----------------------------------------------------------------*/

void
DisjointBoxLayout::initMPI(int argc, const char* argv[])
{
#ifdef USE_MPI
  MPI_Init(&argc, const_cast<char***>(&argv));
  MPI_Comm_size(MPI_COMM_WORLD, &s_numProc);
  MPI_Comm_rank(MPI_COMM_WORLD, &s_procID);
#ifndef NO_CGNS
  cgp_mpi_comm(MPI_COMM_WORLD);
#endif
#endif
}

/*--------------------------------------------------------------------*/
//  Finalize MPI
/** Any application or test using MPI must call this routine when
 *  finished with MPI
 *//*-----------------------------------------------------------------*/

void
DisjointBoxLayout::finalizeMPI()
{
#ifdef USE_MPI
  MPI_Finalize();
#endif
}
