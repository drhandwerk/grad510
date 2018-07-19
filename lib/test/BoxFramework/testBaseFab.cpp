#include <iostream>
#include <iomanip>
#include <vector>

#include "BaseFab.H"
#include "BoxIterator.H"

int main(const int argc, const char* argv[])
{
  const bool verbose = ((argc == 2) && (std::strcmp(argv[1], "-v") == 0));
  const char* const statLbl[] = {
    "failed",
    "passed"
  };
  int status = 0;

//--Tests

  const int testSizeA = D_TERM(3, *3, *3)*2;
  Box boxA(IntVect(D_DECL(0, 0, 0)), IntVect(D_DECL(2, 2, 2)));
  // Test weak construction and define
  {
    FArrayBox fabW;
#if 1
    if (!fabW.box().isEmpty()) {std::cout << "isEmpty" << std::endl; ++status;}
    // Weak construction   
    fabW.define(boxA, 2, -1.0);
    if (fabW(boxA.loVect(), 0) != -1.) {std::cout << "weak1" << std::endl; ++status;}
    if (fabW(boxA.hiVect(), 1) != -1.) {std::cout << "weak2" << std::endl; ++status;}
    // Size should be box size * number of components

    if (fabW.size() != testSizeA) {std::cout << "size1" << std::endl; ++status;}
    if (fabW.size() != boxA.size()*fabW.ncomp()) {std::cout << "size2" << std::endl; ++status;}
    if (fabW.sizeBytes() != boxA.size()*fabW.ncomp()*sizeof(Real)) {std::cout << "size3" << std::endl; ++status;}
    // std::cout << fabW.sizeBytes() << " = " << sizeof(Real) << std::endl;

    // Test stride of components (should be box size)
    if (((&fabW(IntVect(D_DECL(1, 1, 1)), 1)) -
         (&fabW(IntVect(D_DECL(1, 1, 1)), 0))) != boxA.size())
      {
        {std::cout << "stride" << std::endl; ++status;}
      }

    // Test indexing
    if (fabW.index(boxA.loVect()) != 0) {std::cout << "index1" << std::endl; ++status;}
    if (fabW.index(boxA.hiVect()) != (testSizeA/2-1)) {std::cout << "index2:" << fabW.index(boxA.hiVect()) 
                                                       << ' ' << testSizeA/2-1 << std::endl; ++status;}
    int testIdx = (testSizeA/2)/3;
    if (g_SpaceDim == 3)
      {
        if (fabW.index(IntVect(D_DECL(0, 0, 1))) != testIdx) {std::cout << "index3" << std::endl; ++status;}
        testIdx /= 3;
      }
    if (fabW.index(IntVect(D_DECL(0, 1, 0))) != testIdx) {std::cout << "index4" << std::endl; ++status;}
    testIdx /= 3;
    if (fabW.index(IntVect(D_DECL(1, 0, 0))) != testIdx) {std::cout << "index5" << std::endl; ++status;}
#endif
  }

#if 1
  // Test construction
  FArrayBox fabA(boxA, 2, -1.0);
  if (fabA(boxA.loVect(), 0) != -1.) {std::cout << "C1" << std::endl; ++status;}
  if (fabA(boxA.hiVect(), 1) != -1.) {std::cout << "C2" << std::endl; ++status;}
  // Size should be box size * number of components
  if (fabA.size() != testSizeA) {std::cout << "size4" << std::endl; ++status;}
  if (fabA.size() != boxA.size()*fabA.ncomp()) {std::cout << "size5" << std::endl; ++status;}
  // Test stride of components (should be box size)
  if (((&fabA(IntVect(D_DECL(1, 1, 1)), 1)) -
       (&fabA(IntVect(D_DECL(1, 1, 1)), 0))) != boxA.size())
    {
      {std::cout << "stride2" << std::endl; ++status;}
    }
  // Test indexing
  if (fabA.index(boxA.loVect()) != 0) {std::cout << "indexing" << std::endl; ++status;}
  if (fabA.index(boxA.hiVect()) != (testSizeA/2-1)) {std::cout << "indexing" << std::endl; ++status;}
  int testIdx = (testSizeA/2)/3;
  if (g_SpaceDim == 3)
    {
      if (fabA.index(IntVect(D_DECL(0, 0, 1))) != testIdx) {std::cout << "Indexing" << std::endl; ++status;}
      testIdx /= 3;
    }
  if (fabA.index(IntVect(D_DECL(0, 1, 0))) != testIdx) {std::cout << "INdexing" << std::endl; ++status;}
  testIdx /= 3;
  if (fabA.index(IntVect(D_DECL(1, 0, 0))) != testIdx) {std::cout << "Indexing" << std::endl; ++status;}

  // Test assignment
  fabA.setVal(0.);
  if (fabA(boxA.loVect(), 0) != 0.) {std::cout << "ass1" << std::endl; ++status;}
  if (fabA(boxA.hiVect(), 1) != 0.) {std::cout << "ass2" << std::endl; ++status;}
  fabA.setVal(1, 2.);
  if (fabA(boxA.loVect(), 0) != 0.) {std::cout << "ass3" << std::endl; ++status;}
  if (fabA(boxA.hiVect(), 0) != 0.) {std::cout << "ass4" << std::endl; ++status;}
  if (fabA(boxA.loVect(), 1) != 2.) {std::cout << "ass5" << std::endl; ++status;}
  if (fabA(boxA.hiVect(), 1) != 2.) {std::cout << "ass6" << std::endl; ++status;}
  fabA(IntVect(D_DECL(1, 1, 1)), 0) = 5.5;
  if (fabA(IntVect(D_DECL(1, 1, 1)), 0) != 5.5) {std::cout << "ass7" << std::endl; ++status;}

  // Test move assignment
  {
    FArrayBox fabB(boxA, 2, -3.);
    if (fabB.size() != boxA.size()*2) {std::cout << "move1" << std::endl; ++status;}
    Real* data = fabB.dataPtr();
    FArrayBox fabC(boxA, 3, -4.);
    if (fabC.size() != boxA.size()*3) {std::cout << "move2" << std::endl; ++status;}
    fabC = std::move(fabB);
    if (fabC.size() != boxA.size()*2) {std::cout << "move3" << std::endl; ++status;}
    if (fabC.dataPtr() != data) ++status;
    if (fabC(boxA.loVect(), 0) != -3.) {std::cout << "move4" << std::endl; ++status;}
    if (fabC(boxA.hiVect(), 1) != -3.) ++status;
    if (fabB.dataPtr() != nullptr) {std::cout << "move5" << std::endl; ++status;}
  }

  // Try an integer
  {
    const int testSizeB = D_TERM(4, *2, *4)*2;
    Box boxB(IntVect(D_DECL(-2, 0, 2)), IntVect(D_DECL(1, 1, 5)));
    BaseFab<int> fabB(boxB, 2, -1);
    if (fabB(boxB.loVect(), 0) != -1) {std::cout << "1: -1 = " << fabB(boxB.loVect(), 0) << std::endl; ++status;}
    if (fabB(boxB.hiVect(), 1) != -1) {std::cout << "2" << std::endl; ++status;}
    // Size should be box size * number of components
    if (fabB.size() != testSizeB) {std::cout << "3" << std::endl; ++status;}
    if (fabB.size() != boxB.size()*fabB.ncomp()) {std::cout << "" << std::endl; ++status;}
    // Test stride of components (should be box size)
    if (((&fabB(IntVect(D_DECL(1, 1, 3)), 1)) -
         (&fabB(IntVect(D_DECL(1, 1, 3)), 0))) != boxB.size())
      {
        {std::cout << "" << std::endl; ++status;}
      }
    // Test assignment
    fabB.setVal(0);
    if (fabB(boxB.loVect(), 0) != 0) {std::cout << "4" << std::endl; ++status;}
    if (fabB(boxB.hiVect(), 1) != 0) {std::cout << "5" << std::endl; ++status;}
    fabB.setVal(1, 2);
    if (fabB(boxB.loVect(), 0) != 0) {std::cout << "6" << std::endl; ++status;}
    if (fabB(boxB.hiVect(), 0) != 0) {std::cout << "7" << std::endl; ++status;}
    if (fabB(boxB.loVect(), 1) != 2) {std::cout << "8: 2 = " << fabB(boxB.loVect(), 1) << std::endl; ++status;}
    if (fabB(boxB.hiVect(), 1) != 2) {std::cout << "9: 2 = " << fabB(boxB.hiVect(), 1) << std::endl; ++status;}
    fabB(IntVect(D_DECL(1, 1, 3)), 0) = 5;
    if (fabB(IntVect(D_DECL(1, 1, 3)), 0) != 5) {std::cout << "10" << std::endl; ++status;}
  }
#if 1
  // Test simple copy
  {
    int statusSC = 0;
    BaseFab<int> fabC(boxA, 1, 8);
    Box boxC(IntVect(D_DECL(0, 0, 1)), IntVect(D_DECL(2, 0, 2)));
    BaseFab<int> fabD(boxA, 1, 1);
    fabC.copy(boxC, fabD);
    for (BoxIterator bit(boxA); bit.ok(); ++bit)
      {
        const IntVect &iv = *bit;
        if (boxC.contains(iv))
          {
            if (fabC(iv, 0) != 1) ++statusSC;
          }
        else
          {
            if (fabC(iv, 0) != 8) ++statusSC;
          }
        
      }
    if (verbose || statusSC != 0)
      {
        std::cout << "Simple copy test " << statLbl[(statusSC == 0)]
                  << std::endl;
        std::cout << "statusSC: " << statusSC << std::endl;
      }
    status += statusSC;
  }
  
  // Test advanced copy
  {
    int statusAC = 0;
    BaseFab<int> fabC(boxA, 3, 8);
    Box boxC(IntVect(D_DECL(0, 0, 1)), IntVect(D_DECL(2, 0, 2)));
    BaseFab<int> fabD(boxA, 2);
    fabD.setVal(0, 1);
    fabD.setVal(1, 2);
    // Special value
    const IntVect specialC(D_DECL(2, 0, 1));
    const IntVect specialD(D_DECL(2, 2, 1));
    fabD(specialD, 1) = 3;
    Box boxD(IntVect(D_DECL(0, 2, 1)), IntVect(D_DECL(2, 2, 2)));
    fabC.copy(boxC, 1, fabD, boxD, 0, 2);
    for (BoxIterator bit(boxA); bit.ok(); ++bit)
      {
        const IntVect &iv = *bit;
        if (fabC(iv, 0) != 8) ++statusAC;
        if (iv == specialC)
          {
            if (fabC(iv, 1) != 1) ++statusAC;
            if (fabC(iv, 2) != 3) ++statusAC;
          }
        else if (boxC.contains(iv))
          {
            if (fabC(iv, 1) != 1) ++statusAC;
            if (fabC(iv, 2) != 2) ++statusAC;
          }
        else
          {
            if (fabC(iv, 1) != 8) ++statusAC;
            if (fabC(iv, 2) != 8) ++statusAC;
          }
      }
    if (verbose || statusAC != 0)
      {
        std::cout << "Advanced copy test " << statLbl[(statusAC == 0)]
                  << std::endl;
      }
    status += statusAC;
  }
#endif
#endif

  // Test linearout and linear in
#if 1
  {
    int statusLL = 0;
    Box boxB(boxA);
    boxB.grow(1);
    fabA.setVal(-1.2);
    FArrayBox fabB(boxB, 2, -2.3);
    for (BoxIterator bit(boxA); bit.ok(); ++bit)
      {
        const IntVect& iv = *bit;
        fabB(iv, 0) = D_TERM(1000*iv[0], + 100*iv[1], + 10*iv[2]) + 0;
        fabB(iv, 1) = D_TERM(1000*iv[0], + 100*iv[1], + 10*iv[2]) + 1;
      }
    const Box bufferRegion = boxA.adjBox(-1, 0, 0);
    const int bufferSize = bufferRegion.size()*2;
    std::vector<Real> buffer(bufferSize);
    fabB.linearOut(buffer.data(), bufferRegion, 0, 2);
    buffer[0] = -5.6;
    fabA.linearIn(buffer.data(), bufferRegion, 0, 2);
    {
      BoxIterator bit(boxA);
      if (fabA(*bit, 0) != (Real)-5.6) ++statusLL;
      if (fabA(*bit, 1) != D_TERM(  1000*((*bit)[0]),
                                  + 100*((*bit)[1]),
                                  + 10*((*bit)[2])) + 1)
        {
          ++statusLL;
        }
      for (++bit; bit.ok(); ++bit)
        {
          const IntVect& iv = *bit;
          if (fabA(iv, 0) != D_TERM(1000*iv[0] + 0, + 100*iv[1], + 10*iv[2]))
            ++statusLL;
          if (fabA(iv, 1) != D_TERM(1000*iv[0] + 1, + 100*iv[1], + 10*iv[2]))
            ++statusLL;
        }
    }
    if (verbose || statusLL != 0)
      {
        std::cout << "Linear in/out test " << statLbl[(statusLL == 0)]
                  << std::endl;
      }
    status += statusLL;
    //std::cout << "statusLL: " << statusLL << std::endl;
  }
#endif

//--Output status

  if (verbose)
    {
      std::cout << "Status: " << status << std::endl;
    }
  const char* const testName = "testBaseFab";
  std::cout << std::left << std::setw(40) << testName
            << statLbl[(status == 0)] << std::endl;
  return status;
}
