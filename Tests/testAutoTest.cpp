/******************************************************************************
 *
 * # AutoTest: specification-based testing of the compiler
 *
`* Generates random QPULib programs, runs them on the both
 * source language interpreter and the target language emulator, and
 * checks for equivalence.  Currently, it only works in emulation mode.
 *
 ******************************************************************************/
#include "catch.hpp"
#include "QPULib.h"
#include "Common/Seq.h"
#include "Source/Gen.h"
#include "Source/Pretty.h"

using namespace QPULib;

// ============================================================================
// Program-generator options
// ============================================================================

GenOptions basicGenOpts()
{
  GenOptions opts;
  opts.depth           = 3;
  opts.length          = 4;
  opts.numIntArgs      = 4;
  opts.numFloatArgs    = 0;
  opts.numPtrArgs      = 0;
  opts.numPtr2Args     = 0;
  opts.numIntVars      = 4;
  opts.numFloatVars    = 0;
  opts.loopBound       = 5;
  opts.genFloat        = false;
  opts.genRotate       = false;
  opts.genDeref        = false;
  opts.genDeref2       = false;
  opts.derefOffsetMask = 0;
  opts.genStrided      = false;
  return opts;
}

// ============================================================================
// Helpers
// ============================================================================

void printCharSeq(Seq<char>* s)
{
  for (int i = 0; i < s->numElems; i++)
    printf("%c", s->elems[i]);
}

// ============================================================================
// Main
// ============================================================================

TEST_CASE("Interpreter and emulator should work the same", "[autotest]") {
	SECTION("Should generate the same output") {
	  // Seed random generator
	  srand(0);

	  // Basic options
	  GenOptions opts = basicGenOpts();

	  const int numTests = 2000; // Originally 10000, was a bit steep

	  for (int test = 0; test < numTests; test++) {
	    astHeap.clear();
	    resetFreshLabelGen();

	    int numVars, numEmuVars;
	    Stmt* s = progGen(&opts, &numVars);
	    //pretty(s);

	    Seq<Instr> targetCode;
	    resetFreshVarGen(numVars);
	    compileKernel(&targetCode, s);
	    numEmuVars = getFreshVarCount();
	    Seq<int32_t> params;
	    params.clear();
	    for (int i = 0; i < opts.numIntArgs; i++) {
  	    params.append(genIntLit());
	    }

	    Seq<char> interpOut, emuOut;
	    interpreter(1, s, numVars, &params, getBufferObject(), &interpOut);
	    emulate(1, &targetCode, numEmuVars, &params, getBufferObject(), &emuOut);

	    bool differs = false;
	    if (interpOut.numElems != emuOut.numElems)
	      differs = true;
	    else {
	      for (int i = 0; i < interpOut.numElems; i++)
	        if (interpOut.elems[i] != emuOut.elems[i]) { differs = true; break; }
	    }
	
	    if (differs) {
	      printf("Failed test %i.\n", test);
	      pretty(s);
	      printf("Params: ");
	      for (int i = 0; i < params.numElems; i++) {
	        printf("%i ", params.elems[i]);
	      }
	      printf("\nTarget emulator says:\n");
	      printCharSeq(&emuOut);
	      printf("\nSource interpreter says:\n");
	      printCharSeq(&interpOut);
	      printf("\n");
	    }
	    else
	      printf("AutoTest iteration: %i\r", test);

			REQUIRE(!differs);
  	}
	
	  //printf("OK, passed %i tests\n", numTests);
	}
}
