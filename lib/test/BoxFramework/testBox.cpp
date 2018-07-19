#include <cstring>
#include <iostream>
#include <iomanip>

#include "Box.H"

int main(const int argc, const char* argv[])
{
  const bool verbose = ((argc == 2) && (std::strcmp(argv[1], "-v") == 0));
  int status = 0;

//--Tests

  const int testSize = D_TERM(3, *3, *3);
  // Test construction
  const Box boxA(IntVect(D_DECL(0, 0, 0)), IntVect(D_DECL(2, 2, 2)));
  if (verbose)
    {
      std::cout << "boxA.size(): " << boxA.size() << std::endl;
    }
  if (boxA.size() != testSize) ++status;
  // Test copy
  Box boxB(boxA);
  if (boxB.size() != testSize) ++status;
  // Test assignment
  Box boxC;
  boxC = boxB;
  if (boxC.size() != testSize) ++status;

#if 1
  // Test access
  boxC = boxA;
  boxC.loVect(0) = -1;
  boxC.loVect(1) = -2;
  boxC.hiVect(0) = 3;
  boxC.hiVect(1) = 4;
  if (boxC.size() != D_SELECT(5, 35, 105)) {std::cout << "size failed" << std::endl; ++status;}
  if (boxC.loVect() != IntVect(D_DECL(-1, -2, 0))) {std::cout << "loVect failed" << std::endl; ++status;}
  if (boxC.hiVect() != IntVect(D_DECL( 3,  4, 2))) {std::cout << "hiVect failed" << std::endl; ++status;}

  // Test empty
  {
    Box boxEmpty;
    if (boxEmpty.isEmpty() == false) {std::cout << "isEmpty1 failed" << std::endl; ++status;}
    IntVect& lo = const_cast<IntVect&>(boxEmpty.loVect());
    lo = IntVect::Unit;
    IntVect& hi = const_cast<IntVect&>(boxEmpty.hiVect());
    for (int i = 0; i != g_SpaceDim; ++i)
      {
        hi = IntVect::Unit;
        boxEmpty.hiVect(i) = 0;
        if (boxEmpty.isEmpty() == false) {std::cout << "isEmpty2 failed" << std::endl; ++status;}
      }
  }

  // Test grow
  boxB.grow(1);
  if (boxB.size() != (5*IntVect::Unit).product()) {std::cout << "grow(1) failed" << std::endl; ++status;}
  D_EXPR(boxB.grow(-1, 0), boxB.grow(-1, 1), boxB.grow(-1, 2));
  if (boxB.size() != testSize) {std::cout << "grow(size, dir) failed" << std::endl; ++status;}

  // Test shifting and intersection
  boxB.shift(IntVect::Unit);
  boxC = boxA;
  boxC &= boxB;
  if ((boxC.loVect() != IntVect::Unit) || (boxC.hiVect() != (2*IntVect::Unit)))
    {std::cout << "shift1 failed" << std::endl; ++status;}
  boxB.shift(-1, 1);
  boxC = boxA;
  boxC &= boxB;
  if ((boxC.loVect() != IntVect(D_DECL(1, 0, 1))) ||
      (boxC.hiVect() != IntVect(D_DECL(2, 2, 2))))
    {std::cout << "shift2 failed" << std::endl; ++status;}

  // Test adjacent boxes
  boxB = boxA;
  boxB.adjBox(2, 0, -1);
  if (boxB != Box(IntVect(D_DECL(-2, 0, 0)), IntVect(D_DECL(-1, 2, 2))))
    {std::cout << "adjBox1 failed" << std::endl; ++status;}
  boxB = boxA;
  boxB.adjBox(-2, 0, -1);
  if (boxB != Box(IntVect(D_DECL(0, 0, 0)), IntVect(D_DECL(1, 2, 2))))
    {std::cout << "adjBox2 failed" << std::endl; ++status;}
  boxB = boxA;
  boxB.adjBox(2, 1, 1);
  if (boxB != Box(IntVect(D_DECL(0, 3, 0)), IntVect(D_DECL(2, 4, 2))))
    {std::cout << "adjBox3 failed" << std::endl; ++status;}
  boxB = boxA;
  boxB.adjBox(-2, 1, 1);
  if (boxB != Box(IntVect(D_DECL(0, 1, 0)), IntVect(D_DECL(2, 2, 2))))
    {std::cout << "adjBox4 failed" << std::endl; ++status;}

  // Test dimensions 
  boxB = boxA;
  if (boxB.dimensions() != IntVect(D_DECL(3,3,3))) 
    {std::cout << "dimensions failed" << std::endl; ++status;}
  // Grow the upper corner in all directions
  boxB = boxA;
  boxB.growHi(2);
  if(boxB != Box(IntVect(D_DECL(0, 0, 0)), IntVect(D_DECL(4, 4, 4)))) 
    {std::cout << "growHi1 failed" << std::endl; ++status;}

  // Grow the lower side in a specific direction (2D or 3D TEST ONLY)
  boxB = boxA;
  boxB.growLo(2, 1);
  if(boxB != Box(IntVect(D_DECL(0, -2, 0)), IntVect(D_DECL(2, 2, 2))))
    {std::cout << "growLo failed" << std::endl; ++status;}

  // Grow the upper side in a specific direction
  boxB = boxA;
  boxB.growHi(2, 1);
  if(boxB != Box(IntVect(D_DECL(0, 0, 0)), IntVect(D_DECL(2, 4, 2))))
    {std::cout << "growHi2 failed" << std::endl; ++status;}

  // Contains an IntVect
  boxB = boxA;
  if(boxB.contains(IntVect(D_DECL(2, 1, 0))) != true)
    {std::cout << "contains(IV)1 failed" << std::endl; ++status;}
  if(boxB.contains(IntVect(D_DECL(1, 2, 1))) != true)
    {std::cout << "contains(IV)2 failed" << std::endl; ++status;}
  // Contains another box
  boxB = boxA;
  boxC = boxB.grow(-1);
  boxB = boxA;
  if(boxB.contains(boxC) != true)
    {std::cout << "contains(box) failed" << std::endl; ++status;}
#endif

//--Output status

  if (verbose)
    {
      std::cout << "Status: " << status << std::endl;
    }
  const char* const testName = "testBox";
  const char* const statLbl[] = {
    "failed",
    "passed"
  };
  std::cout << std::left << std::setw(40) << testName
            << statLbl[(status == 0)] << std::endl;
  return status;
}
