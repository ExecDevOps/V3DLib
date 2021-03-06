#include "catch.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <V3DLib.h>
#include "support/support.h"

using namespace V3DLib;
using namespace std;


//=============================================================================
// Helper methods
//=============================================================================

template<typename Array>
string showResult(Array &result, int index) {
  ostringstream buf;

  buf << "result  : ";
  for (int j = 0; j < 16; j++) {
    buf << result[16*index + j] << " ";
  }
  buf << "\n";

  return buf.str();
}


template<typename T>
string showExpected(const std::vector<T> &expected) {
  ostringstream buf;

  buf << "expected: ";
  for (int j = 0; j < 16; j++) {
    buf << expected[j] << " ";
  }
  buf << "\n";

  return buf.str();
}


//=============================================================================
// Kernel definition(s)
//=============================================================================

void out(Int &res, Ptr<Int> &result) {
  *result = res;
  result = result + 16;
}


void test(Cond cond, Ptr<Int> &result) {
  Int res = -1;  // temp variable for result of condition, -1 is unexpected value

  If (cond)
    res = 1;
  Else
    res = 0;
  End

  out(res, result);
}


/**
 * @brief Overload for BoolExpr
 *
 * TODO: Why is distinction BoolExpr <-> Cond necessary? Almost the same
 */
void test(BoolExpr cond, Ptr<Int> &result) {
  Int res = -1;  // temp variable for result of condition, -1 is unexpected value

  If (cond)
    res = 1;
  Else
    res = 0;
  End

  out(res, result);
}


void kernel_specific_instructions(Ptr<Int> result) {
  Int a = index();
  Int b = a ^ 1;
  out(b, result);
}


/**
 * @brief Kernel for testing If and When
 */
void kernelIfWhen(Ptr<Int> result) {
  Int outIndex = index();
  Int a = index();

  // any
  test(any(a <   0), result);
  test(any(a <   8), result);
  test(any(a <=  0), result);  // Boundary check
  test(any(a >= 15), result);  // Boundary check
  test(any(a <  32), result);
  test(any(a >  32), result);

  // all
  test(all(a <   0), result);
  test(all(a <   8), result);
  test(all(a <=  0), result);  // Boundary check
  test(all(a >= 15), result);  // Boundary check
  test(all(a <  32), result);
  test(all(a >  32), result);

  // Just If - should be same as any
  test((a <   0), result);
  test((a <   8), result);
  test((a <=  0), result);     // Boundary check
  test((a >= 15), result);     // Boundary check
  test((a <  32), result);
  test((a >  32), result);

  // When
  Int res = -1;  // temp variable for result of condition, -1 is unexpected value
  Where (a < 0) res = 1; Else res = 0; End
  out(res, result);

  res = -1;
  Where (a <= 0) res = 1; Else res = 0; End  // Boundary check
  out(res, result);

  res = -1;
  Where (a >= 15) res = 1; Else res = 0; End  // Boundary check
  out(res, result);

  res = -1;
  Where (a < 8) res = 1; Else res = 0; End
  out(res, result);

  res = -1;
  Where (a >= 8) res = 1; Else res = 0; End
  out(res, result);

  res = -1;
  Where (a < 32) res = 1; Else res = 0; End
  out(res, result);

  res = -1;
  Where (a > 32) res = 1; Else res = 0; End
  out(res, result);
}


template<typename T>
void check_vector(SharedArray<T> &result, int index, std::vector<T> const &expected) {
  REQUIRE(expected.size() == 16);

  bool passed = true;
  int j = 0;
  for (; j < 16; ++j) {
    if (result[16*index + j] != expected[j]) {
      passed = false;
      break;
    }
  }

  INFO("j: " << j);
  INFO(showResult(result, index) << showExpected(expected));
  REQUIRE(passed);
}


void check_conditionals(SharedArray<int> &result, int N) {
  vector<int> allZeroes = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  vector<int> allOnes   = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};

  auto assertResult = [N] ( SharedArray<int> &result, int index, std::vector<int> const &expected) {
    INFO("index: " << index);
    REQUIRE(result.size() == (unsigned) N*16);
    check_vector(result, index, expected);
  };

  // any
  assertResult(result,  0, allZeroes);
  assertResult(result,  1, allOnes);
  assertResult(result,  2, allOnes);
  assertResult(result,  3, allOnes);
  assertResult(result,  4, allOnes);
  assertResult(result,  5, allZeroes);
  // all
  assertResult(result,  6, allZeroes);
  assertResult(result,  7, allZeroes);
  assertResult(result,  8, allZeroes);
  assertResult(result,  9, allZeroes);
  assertResult(result, 10, allOnes);
  assertResult(result, 11, allZeroes);
  // Just If - should be same as any
  assertResult(result, 12, allZeroes);
  assertResult(result, 13, allOnes);
  assertResult(result, 14, allOnes);
  assertResult(result, 15, allOnes);
  assertResult(result, 16, allOnes);
  assertResult(result, 17, allZeroes);
  // where
  assertResult(result, 18, allZeroes);
  assertResult(result, 19, {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0});
  assertResult(result, 20, {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1});
  assertResult(result, 21, {1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0});
  assertResult(result, 22, {0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1});
  assertResult(result, 23, allOnes);
  assertResult(result, 24, allZeroes);
}


class Complex;

namespace V3DLib {

struct ComplexExpr {
  // Abstract syntax tree
  Expr* expr;
  // Constructors
  ComplexExpr();
  //Complex(float x);
};

template <> inline Ptr<Complex> mkArg< Ptr<Complex> >() {
  Ptr<Complex> x;
  x = getUniformPtr<Complex>();
  return x;
}

template <> inline bool passParam< Ptr<Complex>, SharedArray<Complex>* >
  (Seq<int32_t>* uniforms, SharedArray<Complex>* p)
{
  uniforms->append(p->getAddress());
  return true;
}

}


class Complex {
public:
  enum {
    size = 2  // Size of instance in 32-bit values
  };

  Complex() {}

  Complex(const Complex &rhs) : Re(rhs.Re), Im(rhs.Im) {}

  Complex(PtrExpr<Float> input) {
    Re = *input;
    Im = *(input + 1);
  }

  Complex operator *(Complex rhs) {
    Complex tmp;
    tmp.Re = Re*rhs.Re - Im*rhs.Im;
    tmp.Im = Re*rhs.Im + Im*rhs.Re;

    return tmp;
  }

  Complex operator *=(Complex rhs) {
    Complex tmp;

    //FloatExpr tmpRe = Re*rhs.Re - Im*rhs.Im;
    tmp.Re = Re*rhs.Re - Im*rhs.Im;
    tmp.Im = Re*rhs.Im + Im*rhs.Re;

    return tmp;
  }

  Float Re;
  Float Im;
};


void kernelComplex(Ptr<Float> input, Ptr<Float> result) {
  auto inp = input + 2*index();
  auto out = result + 2*index();

  //Complex a(input + 2*index());
  Complex a;
  a.Re = *inp;
  a.Im = *(inp + 1);
  Complex b = a*a;
  *out = b.Re;
  *(out + 1) = b.Im;
}


//=============================================================================
// Unit tests
//=============================================================================

TEST_CASE("Test correct working DSL", "[dsl]") {
  const int N = 25;  // Number of expected result vectors

  SECTION("Test specific instructions") {
    int const NUM = 1;
    vector<int> expected = {1,0,3,2,5,4,7,6,9,8,11,10,13,12,15,14};

    auto k = compile(kernel_specific_instructions);
    //k.pretty(true);

    SharedArray<int> result(16*NUM);

    result.fill(-2);  // Initialize to unexpected value
     k.load(&result).emu();
    check_vector(result, 0, expected);

    result.fill(-2);
     k.load(&result).interpret();
    check_vector(result, 0, expected);

    result.fill(-2);
     k.load(&result).call();
    check_vector(result, 0, expected);
  }


  //
  // Test all variations of If and When
  //
  SECTION("Conditionals work as expected") {
    // Construct kernel
    auto k = compile(kernelIfWhen);

    SharedArray<int> result(16*N);

    // Reset result array to unexpected values
    auto reset = [&result] () {
      result.fill(-2);  // Initialize to unexpected value
    };

    //
    // Run kernel in the three different run modes
    //
    reset();
    k.load(&result).call();
    check_conditionals(result, N);

    reset();
    k.load(&result).emu();
    check_conditionals(result, N);

    reset();
    k.load(&result).interpret();
    check_conditionals(result, N);
  }
}


TEST_CASE("Test construction of composed types in DSL", "[dsl]") {

  SECTION("Test Complex composed type") {
    // TODO: No assertion in this part, need any?

    const int N = 1;  // Number Complex items in vectors

    // Construct kernel
    auto k = compile(kernelComplex);

    // Allocate and array for input and result values
    SharedArray<float> input(2*16*N);
    input[ 0] = 1; input[ 1] = 0;
    input[ 2] = 0; input[ 3] = 1;
    input[ 3] = 1; input[ 4] = 1;

    SharedArray<float> result(2*16*N);

    // Run kernel
    k.load(&input, &result).call();

    //cout << showResult(result, 0) << endl;
  }
}


//-----------------------------------------------------------------------------
// Test for specific DSL operations.
//-----------------------------------------------------------------------------

void int_ops_kernel(Ptr<Int> result) {
  Int a = index();
  a += 3;

  *result = a;
}


void float_ops_kernel(Ptr<Float> result) {
  Float a = toFloat(index());
  a += 3.0f;
  a += 0.25f;

  *result = a;
}


TEST_CASE("Test specific operations in DSL", "[dsl][ops]") {
  SECTION("Test integer operations") {
    int const N = 1;  // Number of expected results

    auto k = compile(int_ops_kernel);

    SharedArray<int> result(16*N);

    k.load(&result).qpu();

    vector<int> expected = {3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18};
    check_vector(result, 0, expected);
  }


  SECTION("Test float operations") {
    int const N = 1;  // Number of expected results

    auto k = compile(float_ops_kernel);

    SharedArray<float> result(16*N);

    k.load(&result).qpu();

    vector<float> expected = { 3.25,  4.25,  5.25,  6.25,  7.25,  8.25,  9.25, 10.25,
                              11.25, 12.25, 13.25, 14.25, 15.25, 16.25, 17.25, 18.25};
    //dump_array(result);
    check_vector(result, 0, expected);
  }
}


void nested_for_kernel(Ptr<Int> result) {
  int const COUNT = 3;
  Int x = 0;

  For (Int n = 0, n < COUNT, n++)
    For (Int m = 0, m < COUNT, m++)
      x += 1;

      Where ((index() & 0x1) == 1)
        x += 1;
      End

      If ((m & 0x1) == 1)
        x += 1;
      End
    End

    x += 2;
  End

  *result = x;
}


TEST_CASE("Test For-loops", "[dsl][for]") {
  Platform::use_main_memory(true);

  SECTION("Test nested For-loops") {
    auto k = compile(nested_for_kernel);

    SharedArray<int> result(16);
    k.load(&result).emu();
    //dump_array(result);

    vector<int> expected = {18, 27, 18, 27, 18, 27, 18, 27, 18, 27, 18, 27, 18, 27, 18, 27};
    check_vector(result, 0, expected);
  }

  Platform::use_main_memory(false);
} 
