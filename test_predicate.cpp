/*
 * This is free and unencumbered software released into the public domain.
 *
 * Anyone is free to copy, modify, publish, use, compile, sell, or
 * distribute this software, either in source code form or as a compiled
 * binary, for any purpose, commercial or non-commercial, and by any
 * means.
 *
 * In jurisdictions that recognize copyright laws, the author or authors
 * of this software dedicate any and all copyright interest in the
 * software to the public domain. We make this dedication for the benefit
 * of the public at large and to the detriment of our heirs and
 * successors. We intend this dedication to be an overt act of
 * relinquishment in perpetuity of all present and future rights to this
 * software under copyright law.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * For more information, please refer to <https://unlicense.org>
 */

#include <cstdio>

#define PKO_PREDICATE_LOGGER_HELPER(result, str)        \
{                                                       \
  fprintf(stderr, "%s\n", str);                         \
}

#include <predicate.hpp>

#include <array>
#include <vector>

using namespace pajko;
using namespace pajko::Predicate;

template <typename T>
bool testfunc(const Encapsulator& arg)
{
  auto& val = *(reinterpret_cast<const T*>(arg.fetch()));
  return PKO_PREDICATE_LOGGER(val == 42, val << " is not fourtytwo");
}

template <typename T>
bool iseven(const Encapsulator& arg)
{
  auto& val = *(reinterpret_cast<const T*>(arg.fetch()));
  return PKO_PREDICATE_LOGGER((val % 2) == 0, val << " is not even");
}

template <typename T>
bool isgt10(const Encapsulator& arg)
{
  auto& val = *(reinterpret_cast<const T*>(arg.fetch()));
  return PKO_PREDICATE_LOGGER(val > 10, val << " is not greater than ten");
}

template <typename T>
bool isbetween(const Encapsulator& arg, T low, T high)
{
  auto& val = *(reinterpret_cast<const T*>(arg.fetch()));
  return PKO_PREDICATE_LOGGER(val >= low && val <= high, val << " is not between " << low << " and " << high);
}

#define evalhelper(p) \
  printf("%s = %u\n", #p, p)

int main(void) noexcept
{
  uint8_t bufexpected[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
  uint8_t bufactual[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 10 };

  int bufexpected2[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

  std::array<uint8_t, 10> arr{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 10 };
  std::vector<uint8_t> vec{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 10 };

  evalhelper(PredicateExecHelper(IsEqual(bufexpected), Encapsulate(bufactual)) == 0);
  evalhelper(PredicateExecHelper(IsEqual(bufactual), Encapsulate(bufactual)) == 1);

  // WARNING: make sure that the type matches (the default is int)
  evalhelper(PredicateExecHelper(IsEqual({0, 1, 2, 3, 4, 5, 6, 7, 8, 9}), Encapsulate(bufexpected2)) == 1);

  // yeah, I know, but this is just a test...
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wc99-extension"
  evalhelper(PredicateExecHelper(IsEqual((uint8_t []){0, 1, 2, 3, 4, 5, 6, 7, 8, 9}), Encapsulate(bufactual)) == 0);
  evalhelper(PredicateExecHelper(IsEqual((uint8_t []){0, 1, 2, 3, 4, 5, 6, 7, 8, 10}), Encapsulate(bufactual)) == 1);
#pragma GCC diagnostic pop

  evalhelper(PredicateExecHelper(IsEqual(std::array<uint8_t, 10>{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 10 }), Encapsulate(arr)) == 1);
  evalhelper(PredicateExecHelper(IsEqual(std::vector<uint8_t>{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 10 }), Encapsulate(vec)) == 1);

  evalhelper(PredicateExecHelper(IsEqual(42), Encapsulate(42)) == 1);
  evalhelper(PredicateExecHelper(IsEqual(42), Encapsulate(24)) == 0);

  evalhelper(PredicateExecHelper(IsOdd<uint8_t>(), Encapsulate<uint8_t>(42)) == 0);
  evalhelper(PredicateExecHelper(IsEven<uint8_t>(), Encapsulate(21)) == 0);
  evalhelper(PredicateExecHelper(IsDivisibleBy<uint8_t>(7), Encapsulate(42)) == 1);
  evalhelper(PredicateExecHelper(IsDivisibleBy<uint8_t>(7), Encapsulate(43)) == 0);
  evalhelper(PredicateExecHelper(InBetween<uint8_t>(10, 20), Encapsulate(15)) == 1);
  evalhelper(PredicateExecHelper(InBetween<uint8_t>(10, 20), Encapsulate(42)) == 0);
  evalhelper(PredicateExecHelper(Outside<uint8_t>(10, 20), Encapsulate(42)) == 1);
  evalhelper(PredicateExecHelper(Outside<uint8_t>(10, 20), Encapsulate(15)) == 0);
  evalhelper(PredicateExecHelper(IsEqualEpsilon<uint8_t>(15, 2), Encapsulate(12)) == 0);
  evalhelper(PredicateExecHelper(IsEqualEpsilon<uint8_t>(15, 2), Encapsulate(13)) == 1);
  evalhelper(PredicateExecHelper(IsEqualEpsilon<uint8_t>(15, 2), Encapsulate(14)) == 1);
  evalhelper(PredicateExecHelper(IsEqualEpsilon<uint8_t>(15, 2), Encapsulate(15)) == 1);
  evalhelper(PredicateExecHelper(IsEqualEpsilon<uint8_t>(15, 2), Encapsulate(16)) == 1);
  evalhelper(PredicateExecHelper(IsEqualEpsilon<uint8_t>(15, 2), Encapsulate(17)) == 1);
  evalhelper(PredicateExecHelper(IsEqualEpsilon<uint8_t>(15, 2), Encapsulate(18)) == 0);

  evalhelper(PredicateExecHelper(IsEqualEpsilon<double>(0.2, 0.001), Encapsulate(0.3)) == 0);
  evalhelper(PredicateExecHelper(IsEqualEpsilon<double>(0.2, 0.1), Encapsulate(0.3)) == 1);
  evalhelper(PredicateExecHelper(IsLesserEq<double>(42.5), Encapsulate(102.3)) == 0);
  evalhelper(PredicateExecHelper(IsLesserEq<double>(42.5), Encapsulate(12.3)) == 1);
  evalhelper(PredicateExecHelper(IsOdd<double>(), Encapsulate(102.3)) == 0);
  evalhelper(PredicateExecHelper(IsEven<double>(), Encapsulate(102.3)) == 0);
  evalhelper(PredicateExecHelper(IsOdd<double>(), Encapsulate(102.0)) == 0);
  evalhelper(PredicateExecHelper(IsEven<double>(), Encapsulate(102.0)) == 1);
  evalhelper(PredicateExecHelper(IsOdd<double>(), Encapsulate(103.0)) == 1);
  evalhelper(PredicateExecHelper(IsEven<double>(), Encapsulate(103.0)) == 0);
  evalhelper(PredicateExecHelper(IsZero<double>(), Encapsulate(102.3)) == 0);
  evalhelper(PredicateExecHelper(IsZero<double>(), Encapsulate(0.0)) == 1);
  evalhelper(PredicateExecHelper(IsNonZero<double>(), Encapsulate(102.3)) == 1);
  evalhelper(PredicateExecHelper(InBetween<double>(10.2, 10.8), Encapsulate(10.9)) == 0);
  evalhelper(PredicateExecHelper(IsDivisibleBy<double>(2.5), Encapsulate(5.0)) == 1);
  evalhelper(PredicateExecHelper(InBetween<uint8_t>(10, 20), Encapsulate<uint8_t>(42)) == 0);

  evalhelper(PredicateExecHelper(IsEqual<double>(3.14159), Encapsulate(3.14159)) == 1);

  ForgetPredicates();

  evalhelper(PredicateExecHelper(Obey(testfunc<int>), Encapsulate(41)) == 0);
  evalhelper(PredicateExecHelper(Obey(testfunc<int>), Encapsulate(42)) == 1);
  evalhelper(PredicateExecHelper(Obey(testfunc<int>), Encapsulate(43)) == 0);

  evalhelper(PredicateExecHelper(Resist(testfunc<int>), Encapsulate(41)) == 1);
  evalhelper(PredicateExecHelper(Resist(testfunc<int>), Encapsulate(42)) == 0);
  evalhelper(PredicateExecHelper(Resist(testfunc<int>), Encapsulate(43)) == 1);

  auto allref = MatchAll(isgt10<int>, IsEven<int>());
  evalhelper(PredicateExecHelper(allref, Encapsulate(42)) == 1);
  evalhelper(PredicateExecHelper(allref, Encapsulate(43)) == 0);
  evalhelper(PredicateExecHelper(allref, Encapsulate(8)) == 0);
  evalhelper(PredicateExecHelper(allref, Encapsulate(7)) == 0);

  auto oneref = MatchOne(isgt10<int>, IsEven<int>());
  evalhelper(PredicateExecHelper(oneref, Encapsulate(8)) == 1);
  evalhelper(PredicateExecHelper(oneref, Encapsulate(7)) == 1);
  evalhelper(PredicateExecHelper(oneref, Encapsulate(43)) == 1);
  evalhelper(PredicateExecHelper(oneref, Encapsulate(42)) == 0);

  auto anyref = MatchAny(isgt10<int>, IsEven<int>());
  evalhelper(PredicateExecHelper(anyref, Encapsulate(8)) == 1);
  evalhelper(PredicateExecHelper(anyref, Encapsulate(7)) == 0);
  evalhelper(PredicateExecHelper(anyref, Encapsulate(43)) == 1);
  evalhelper(PredicateExecHelper(anyref, Encapsulate(42)) == 1);

  auto noneref = MatchNone(isgt10<int>, IsEven<int>());
  evalhelper(PredicateExecHelper(noneref, Encapsulate(8)) == 0);
  evalhelper(PredicateExecHelper(noneref, Encapsulate(7)) == 1);
  evalhelper(PredicateExecHelper(noneref, Encapsulate(43)) == 0);
  evalhelper(PredicateExecHelper(noneref, Encapsulate(42)) == 0);

  auto odd_between_10_and_20 = MatchAll(WithArgs(isbetween, 10, 20), IsOdd<int>());

  evalhelper(PredicateExecHelper(odd_between_10_and_20, Encapsulate(42)) == 0);
  evalhelper(PredicateExecHelper(odd_between_10_and_20, Encapsulate(43)) == 0);
  evalhelper(PredicateExecHelper(odd_between_10_and_20, Encapsulate(12)) == 0);
  evalhelper(PredicateExecHelper(odd_between_10_and_20, Encapsulate(13)) == 1);

  evalhelper(PredicateExecHelper(Obey(IsOdd<int>()), Encapsulate(13)) == 1);
  evalhelper(PredicateExecHelper(Obey(IsOdd<int>()), Encapsulate(42)) == 0);
  evalhelper(PredicateExecHelper(Resist(IsOdd<int>()), Encapsulate(13)) == 0);
  evalhelper(PredicateExecHelper(Resist(IsOdd<int>()), Encapsulate(42)) == 1);

  // works with lambda
  evalhelper(PredicateExecHelper(Obey([](const Encapsulator&){ return true; }), Encapsulate(13)) == 1);
  evalhelper(PredicateExecHelper(Obey([](const Encapsulator&){ return false; }), Encapsulate(13)) == 0);

  // hell yeah, no way to get back the type...
  evalhelper(PredicateExecHelper(Obey([](const Encapsulator& e){ return Decapsulate<int>(e) == 42; }), Encapsulate(42)) == 1);
  evalhelper(PredicateExecHelper(Obey([](const Encapsulator& e){ return Decapsulate<int>(e) == 42; }), Encapsulate(13)) == 0);

  ForgetPredicates();

  return 0;
}
