
#ifndef _INTVECT_H_
#define _INTVECT_H_


/******************************************************************************/
/**
 * \file IntVect.H
 *
 * \brief Class for integer vector in space
 *
 *//*+*************************************************************************/

#include <ostream>
#include <array>

#include "Parameters.H"

/*
  Notes:
    - tag __CUDACC__ means the file is process with nvcc.  The class is
      modified whether on the cpu or gpu.
    - tag __CUDA_ARCH__ is defined for device code.  This only works in
      functions with the __device__ qualifier.  It cannot be used in the
      class declaration.
*/

#undef HOSTDEVICE
#undef DEVICE
#ifdef __CUDACC__
#define HOSTDEVICE __host__ __device__
#define DEVICE __device__
#else
#define HOSTDEVICE
#define DEVICE
#endif


/*******************************************************************************
 */
///  Integer vector in space
/**
 ******************************************************************************/

class IntVect
{


/*==============================================================================
 * Public constructors and destructors
 *============================================================================*/

public:

  /// Default constructor
  HOSTDEVICE IntVect();

  /// Construct with values
  HOSTDEVICE IntVect(D_DECL(const int a_i0, const int a_i1, const int a_i2));

//--Use synthesized copy constructor

  /// Copy constructor
  HOSTDEVICE IntVect(const IntVect&) = default;
  // IntVect(const IntVect& a_iv)
  //   : m_data(a_iv.m_data)
  //   { }

//--Use synthesized move constructor

  /// Move constructor
  HOSTDEVICE IntVect(IntVect&&) = default;
  // IntVect(IntVect&& a_iv)
  //   : m_data(std::move(a_iv.m_data))
  //   { }

//--Use synthesized assignment constructor

  /// Assignment constructor
  HOSTDEVICE IntVect& operator=(const IntVect&) = default;
  // IntVect& operator=(const IntVect& a_iv)
  //   {
  //     if (&a_iv != this)
  //       {
  //         m_data = a_iv.m_data;
  //       }
  //     return *this;
  //   }

//--Use synthesized move assignment constructor

  /// Move assignment constructor
  HOSTDEVICE IntVect& operator=(IntVect&&) = default;
  // IntVect& operator=(IntVect&& a_iv)
  //   {
  //     if (&a_iv != this)
  //       {
  //         m_data = std::move(a_iv.m_data);
  //       }
  //     return *this;
  //   }

//--Use synthesized destructor

  /// Destructor
  HOSTDEVICE ~IntVect() = default;
  // ~IntVect()
  //   { } 


/*==============================================================================
 * Private constructors and destructors
 *============================================================================*/

// private:

//--Standard copy not permitted

  // IntVect(const IntVect&);

//--Assignment not permitted

  // IntVect &operator=(const IntVect&);


/*==============================================================================
 * Members functions
 *============================================================================*/

public:

  /// Constant access to an index
  HOSTDEVICE const int& operator[](const int a_idx) const;

  /// Access to an index
  HOSTDEVICE int& operator[](const int a_idx);

  /// Add another IntVect
  HOSTDEVICE IntVect& operator+=(const IntVect& a_iv);

  /// Subtract another IntVect
  HOSTDEVICE IntVect& operator-=(const IntVect& a_iv);

  /// Add scalar
  HOSTDEVICE IntVect& operator+=(const int a_scalar);

  /// Subtract scalar
  HOSTDEVICE IntVect& operator-=(const int a_scalar);

  /// Minimum for each component (modifies this)
  HOSTDEVICE IntVect& min(const IntVect& a_iv);

  /// Maximum for each component (modifies this)
  HOSTDEVICE IntVect& max(const IntVect& a_iv);

  /// Unary negation
  HOSTDEVICE IntVect operator-() const;

  /// One norm
  HOSTDEVICE int norm1() const;

  /// Sum
  HOSTDEVICE int sum() const;

  /// Product
  HOSTDEVICE int product() const;

//--Only for use on GPU

#ifdef __CUDACC__
  /// Constant access to the data
  HOSTDEVICE const int* dataPtr() const;

  /// Access to the data
  HOSTDEVICE int* dataPtr();
#endif


/*==============================================================================
 * Data members
 *============================================================================*/

protected:

#ifdef __CUDACC__
  int m_data[g_SpaceDim];
#else
  std::array<int, g_SpaceDim> m_data; ///< Static array holding the data for
                                      ///< the vector
#endif

public:

#ifndef __CUDACC__
  static const IntVect Zero;          ///< All components 0
  static const IntVect Unit;          ///< All components 1
#endif
};


/*******************************************************************************
 *
 * Class IntVect: inline member definitions
 *
 ******************************************************************************/

#ifdef __CUDACC__
/*--------------------------------------------------------------------*/
//  Default constructor
/*--------------------------------------------------------------------*/

DEVICE inline
IntVect::IntVect()
{
}

/*--------------------------------------------------------------------*/
//  Constructor with values
/** \param[in] a_i#     Initial values
 *//*-----------------------------------------------------------------*/

DEVICE inline
IntVect::IntVect(D_DECL(const int a_i0,
                        const int a_i1,
                        const int a_i2))
{
  D_EXPR(m_data[0] = a_i0, m_data[1] = a_i1, m_data[2] = a_i2);
}
#endif

/*--------------------------------------------------------------------*/
//  Constant access to an index
/** \param[in]  a_idx   Spatial dimension index
 *  \return             Value at requested dimension
 *//*-----------------------------------------------------------------*/

HOSTDEVICE inline const int&
IntVect::operator[](const int a_idx) const
{
  return m_data[a_idx];
}

/*--------------------------------------------------------------------*/
//  Access to an index
/** \param[in]  a_idx   Spatial dimension index
 *  \return             Value at requested dimension
 *//*-----------------------------------------------------------------*/

HOSTDEVICE inline int&
IntVect::operator[](const int a_idx)
{
  return m_data[a_idx];
}

/*--------------------------------------------------------------------*/
//  Pointwise addition
/** \param[in]  a_iv    IntVect to add to this
 *  \return             Sum of coordinates
 *//*-----------------------------------------------------------------*/

HOSTDEVICE inline IntVect&
IntVect::operator+=(const IntVect& a_iv)
{
  D_EXPR(m_data[0] += a_iv[0],
  m_data[1] += a_iv[1],
  m_data[2] += a_iv[2]);
  return *this;
}

/*--------------------------------------------------------------------*/
//  Subtract another IntVect
/** \param[in]  a_iv    IntVect to subtract
 *  \return             Modified value of this IntVect
 *//*-----------------------------------------------------------------*/

HOSTDEVICE inline IntVect&
IntVect::operator-=(const IntVect& a_iv)
{
  D_EXPR(m_data[0] -= a_iv[0],
  m_data[1] -= a_iv[1],
  m_data[2] -= a_iv[2]);
  return *this;
}

/*--------------------------------------------------------------------*/
//  Add scalar
/** \param[in]  a_scalar
 *                      Scalar to add
 *  \return             Modified value of this IntVect
 *//*-----------------------------------------------------------------*/

HOSTDEVICE inline IntVect&
IntVect::operator+=(const int a_scalar)
{
  D_EXPR(m_data[0] += a_scalar,
  m_data[1] += a_scalar,
  m_data[2] += a_scalar);
  return *this;
}

/*--------------------------------------------------------------------*/
//  Subtract scalar
/** \param[in]  a_scalar
 *                      Scalar to subtract
 *  \return             Modified value of this IntVect
 *//*-----------------------------------------------------------------*/

HOSTDEVICE inline IntVect&
IntVect::operator-=(const int a_scalar)
{
  D_EXPR(m_data[0] -= a_scalar,
  m_data[1] -= a_scalar,
  m_data[2] -= a_scalar);
  return *this;
}

/*--------------------------------------------------------------------*/
//  Minimum for each component (modifies this)
/** \param[in]  a_iv    Other IntVect
 *  \return             Modified value of this IntVect
 *//*-----------------------------------------------------------------*/

HOSTDEVICE inline IntVect&
IntVect::min(const IntVect& a_iv)
{

#ifdef __CUDA_ARCH__
  D_EXPR(m_data[0] = ::min(m_data[0], a_iv[0]),
         m_data[1] = ::min(m_data[1], a_iv[1]),
         m_data[2] = ::min(m_data[2], a_iv[2]));
#else
  D_EXPR(m_data[0] = std::min(m_data[0], a_iv[0]),
         m_data[1] = std::min(m_data[1], a_iv[1]),
         m_data[2] = std::min(m_data[2], a_iv[2]));
#endif

  return *this;
}

/*--------------------------------------------------------------------*/
//  Maximum for each component (modifies this)
/** \param[in]  a_iv    Other IntVect
 *  \return             Modified value of this IntVect
 *//*-----------------------------------------------------------------*/

HOSTDEVICE inline IntVect&
IntVect::max(const IntVect& a_iv)
{
#ifdef __CUDA_ARCH__
  D_EXPR(m_data[0] = ::max(m_data[0], a_iv[0]),
         m_data[1] = ::max(m_data[1], a_iv[1]),
         m_data[2] = ::max(m_data[2], a_iv[2]));
#else
  D_EXPR(m_data[0] = std::max(m_data[0], a_iv[0]),
         m_data[1] = std::max(m_data[1], a_iv[1]),
         m_data[2] = std::max(m_data[2], a_iv[2]));
#endif

  return *this;
}

/*--------------------------------------------------------------------*/
//  Unary negation
/** \return             Negate all components
 *//*-----------------------------------------------------------------*/

HOSTDEVICE inline IntVect
IntVect::operator-() const
{
  return IntVect(D_DECL(-m_data[0], -m_data[1], -m_data[2]));
}

/*--------------------------------------------------------------------*/
//  One norm
/** \f$\sum_{j=0}^D|\boldsymbol{i}_j|\f$
 *//*-----------------------------------------------------------------*/

HOSTDEVICE inline int
IntVect::norm1() const
{
  int r_abs = 0;
  D_EXPR(r_abs += std::abs(m_data[0]), r_abs += std::abs(m_data[1]), r_abs += std::abs(m_data[2]));
  return r_abs;
}

/*--------------------------------------------------------------------*/
//  Sum of all components
/*--------------------------------------------------------------------*/

HOSTDEVICE inline int
IntVect::sum() const
{
  int r_sum = 0;
  D_EXPR(r_sum += m_data[0], r_sum += m_data[1], r_sum += m_data[2]);
  return r_sum;
}

/*--------------------------------------------------------------------*/
//  Product of all components
/*--------------------------------------------------------------------*/

HOSTDEVICE inline int
IntVect::product() const
{
  int r_prod = 1;
  D_EXPR(r_prod *= m_data[0], r_prod *= m_data[1], r_prod *= m_data[2]);
  return r_prod;
}

#ifdef __CUDACC__
/*--------------------------------------------------------------------*/
//  Constant access to the data
/*--------------------------------------------------------------------*/

HOSTDEVICE inline const int*
IntVect::dataPtr() const
{
  return m_data;
}

/*--------------------------------------------------------------------*/
//  Access to the data
/*--------------------------------------------------------------------*/

HOSTDEVICE inline int*
IntVect::dataPtr()
{
  return m_data;
}
#endif

/*******************************************************************************
 *
 * Class IntVect: external related functions
 *
 ******************************************************************************/

/// Output the IntVect
std::ostream &operator<<(std::ostream &a_os, const IntVect &a_iv);

/*--------------------------------------------------------------------*/
/// Equal to
/** \param[in]  a_ivx   First IntVect
 *  \param[in]  a_ivy   Second IntVect
 *  \return             T if equal
 *//*-----------------------------------------------------------------*/

HOSTDEVICE inline bool
operator==(const IntVect& a_ivx, const IntVect& a_ivy)
{
  return D_TERM(   (a_ivx[0] == a_ivy[0]),
                && (a_ivx[1] == a_ivy[1]),
                && (a_ivx[2] == a_ivy[2]));
}

/*--------------------------------------------------------------------*/
/// Not equal to
/** \param[in]  a_ivx   First IntVect
 *  \param[in]  a_ivy   Second IntVect
 *  \return             T if not equal
 *//*-----------------------------------------------------------------*/

HOSTDEVICE inline bool
operator!=(const IntVect& a_ivx, const IntVect& a_ivy)
{
  return D_TERM(   (a_ivx[0] != a_ivy[0]),
                || (a_ivx[1] != a_ivy[1]),
                || (a_ivx[2] != a_ivy[2]));
}

/*--------------------------------------------------------------------*/
/// Less than
/** \param[in]  a_ivx   First IntVect
 *  \param[in]  a_ivy   Second IntVect
 *  \return             T if this is < other in all indices
 *//*-----------------------------------------------------------------*/

HOSTDEVICE inline bool
operator<(const IntVect& a_ivx, const IntVect& a_ivy)
{
  return D_TERM(   (a_ivx[0] < a_ivy[0]),
		&& (a_ivx[1] < a_ivy[1]),
		&& (a_ivx[2] < a_ivy[2]));
}

/*--------------------------------------------------------------------*/
/// Less than or equal to
/** \param[in]  a_ivx   First IntVect
 *  \param[in]  a_ivy   Second IntVect
 *  \return             T if this is <= other in all indices
 *//*-----------------------------------------------------------------*/

HOSTDEVICE inline bool
operator<=(const IntVect& a_ivx, const IntVect& a_ivy)
{
  return D_TERM(   (a_ivx[0] <= a_ivy[0]),
                && (a_ivx[1] <= a_ivy[1]),
                && (a_ivx[2] <= a_ivy[2]));
}

/*--------------------------------------------------------------------*/
/// + with IntVect
/** \param[in]  a_ivx   First IntVect
 *  \param[in]  a_ivy   Second IntVect
 *  \return             Sum of IntVect components
 *//*-----------------------------------------------------------------*/

HOSTDEVICE inline IntVect
operator+(const IntVect& a_ivx, const IntVect& a_ivy)
{
  return IntVect(D_DECL(a_ivx[0] + a_ivy[0],
                        a_ivx[1] + a_ivy[1],
                        a_ivx[2] + a_ivy[2]));
}

/*--------------------------------------------------------------------*/
/// - with IntVect
/** \param[in]  a_ivx   First IntVect
 *  \param[in]  a_ivy   Second IntVect
 *  \return             Difference of IntVect components
 *//*-----------------------------------------------------------------*/

HOSTDEVICE inline IntVect
operator-(const IntVect& a_ivx, const IntVect& a_ivy)
{
  return IntVect(D_DECL(a_ivx[0] - a_ivy[0],
                        a_ivx[1] - a_ivy[1],
                        a_ivx[2] - a_ivy[2]));
}

/*--------------------------------------------------------------------*/
/// * with IntVect
/** \param[in]  a_ivx   First IntVect
 *  \param[in]  a_ivy   Second IntVect
 *  \return             Multiplication of IntVect components
 *//*-----------------------------------------------------------------*/

HOSTDEVICE inline IntVect
operator*(const IntVect& a_ivx, const IntVect& a_ivy)
{
  return IntVect(D_DECL(a_ivx[0] * a_ivy[0],
                        a_ivx[1] * a_ivy[1],
                        a_ivx[2] * a_ivy[2]));
}

/*--------------------------------------------------------------------*/
/// / with IntVect
/** \param[in]  a_ivx   First IntVect
 *  \param[in]  a_ivy   Second IntVect
 *  \return             Division of IntVect components
 *//*-----------------------------------------------------------------*/

HOSTDEVICE inline IntVect
operator/(const IntVect& a_ivx, const IntVect& a_ivy)
{
  return IntVect(D_DECL(a_ivx[0] / a_ivy[0],
                        a_ivx[1] / a_ivy[1],
                        a_ivx[2] / a_ivy[2]));
}

/*--------------------------------------------------------------------*/
/// Scaling the IntVect (IntVect * scalar)
/** \param[in]  a_scale Integer scaling
 *  \param[in]  a_iv    IntVect
 *  \return             Scaled IntVect
 *//*-----------------------------------------------------------------*/

HOSTDEVICE inline IntVect
operator*(const IntVect& a_iv, const int a_scale)
{
  return IntVect(D_DECL(a_iv[0] * a_scale,
                        a_iv[1] * a_scale,
                        a_iv[2] * a_scale));
}

/*--------------------------------------------------------------------*/
/// Scaling the IntVect (scalar * IntVect)
/** \param[in]  a_iv    IntVect
 *  \param[in]  a_scale Integer scaling
 *  \return             Scaled IntVect
 *//*-----------------------------------------------------------------*/

HOSTDEVICE inline IntVect
operator*(const int a_scale, const IntVect& a_iv)
{
  return IntVect(D_DECL(a_iv[0] * a_scale,
                        a_iv[1] * a_scale,
                        a_iv[2] * a_scale));
}

#endif  /* ! defined _INTVECT_H_ */
