#include <iostream>

#include "IntVect.H"

int main()
{
  IntVect iv(D_DECL(0,1,2));
  std::cout << iv << std::endl;
  std::cout << "Dimensions: " << g_SpaceDim << std::endl;
}
