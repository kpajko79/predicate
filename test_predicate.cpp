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
 *
 *
 * SPDX-License-Identifier: Unlicense
 */

#include <cstdio>

#define PKO_PREDICATE_LOGGER_HELPER(result, str)        \
{                                                       \
  fprintf(stderr, "%s\n", str);                         \
}

#include <predicate.hpp>

#include <array>
#include <vector>
#include <tuple>

using namespace pajko;
using namespace pajko::Predicate;

template <typename T>
// cppcheck-suppress unusedFunction
bool testfunc(const Encapsulator& arg)
{
  const auto& result = Decapsulate<T>(arg);
  if (!std::get<0>(result))
  {
    return false;
  }

  const auto& val = std::get<1>(result);
  return PKO_PREDICATE_LOGGER(val == 42, val << " is not fourtytwo");
}

template <typename T>
// cppcheck-suppress unusedFunction
bool isgt10(const Encapsulator& arg)
{
  const auto& result = Decapsulate<T>(arg);
  if (!std::get<0>(result))
  {
    return false;
  }

  const auto& val = std::get<1>(result);
  return PKO_PREDICATE_LOGGER(val > 10, val << " is not greater than ten");
}

template <typename T>
bool isbetween(const Encapsulator& arg, T low, T high)
{
  const auto& result = Decapsulate<T>(arg);
  if (!std::get<0>(result))
  {
    return false;
  }

  const auto& val = std::get<1>(result);
  return PKO_PREDICATE_LOGGER(val >= low && val <= high, val << " is not between " << low << " and " << high);
}

template <typename T>
using twople_t = std::tuple<T, T>;

template <typename T>
// cppcheck-suppress unusedFunction
bool sumis15(const Encapsulator &arg)
{
  const auto& result = Decapsulate<twople_t<T>>(arg);
  if (!std::get<0>(result))
  {
    return false;
  }

  const auto& val = std::get<1>(result);
  auto a = std::get<0>(val);
  auto b = std::get<1>(val);
  return PKO_PREDICATE_LOGGER((a + b) == 15, "the sum of " << a << " and " << b << " is not 15");
}

#define evalhelper(p) \
  fprintf(stderr, "%s = %u\n", #p, p)

int main() noexcept
{
  // test type matching (the first one should pass if the types would match)
  { auto pred = Obey(testfunc<int>); evalhelper(PredicateExecHelper(pred, Encapsulate<uint8_t>(42)) == 0); }
  { auto pred = Obey(testfunc<int>); evalhelper(PredicateExecHelper(pred, Encapsulate<uint8_t>(41)) == 0); }

  uint8_t bufexpected[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
  uint8_t bufactual[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 10 };

  int bufexpected2[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

  // cppcheck-suppress variableScope
  std::array<uint8_t, 10> arr{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 10 };
  // cppcheck-suppress variableScope
  std::vector<uint8_t> vec{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 10 };

  { auto pred = IsEqual(bufexpected); evalhelper(PredicateExecHelper(pred, Encapsulate(bufactual)) == 0); }
  { auto pred = IsEqual(bufactual); evalhelper(PredicateExecHelper(pred, Encapsulate(bufactual)) == 1); }
  // type checking should catch this
  { auto pred = IsEqual(bufexpected); evalhelper(PredicateExecHelper(pred, Encapsulate(bufexpected2)) == 0); }

  { auto pred = IsEqual({0, 1, 2, 3, 4, 5, 6, 7, 8, 9}); evalhelper(PredicateExecHelper(pred, Encapsulate(bufexpected2)) == 1); }
  // type check should catch this
  { auto pred = IsEqual({0, 1, 2, 3, 4, 5, 6, 7, 8, 9}); evalhelper(PredicateExecHelper(pred, Encapsulate(bufexpected)) == 0); }
  { auto pred = IsEqual({ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }); evalhelper(PredicateExecHelper(pred, Encapsulate({ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 })) == 1); }
  { auto pred = IsEqual({ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }); evalhelper(PredicateExecHelper(pred, Encapsulate({ 0, 1, 2, 3, 4, 5, 6, 7, 8, 10 })) == 0); }

  // yeah, I know, but this is just a test...
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wc99-extension"
  { auto pred = IsEqual((uint8_t []){0, 1, 2, 3, 4, 5, 6, 7, 8, 9}); evalhelper(PredicateExecHelper(pred, Encapsulate(bufactual)) == 0); }
  { auto pred = IsEqual((uint8_t []){0, 1, 2, 3, 4, 5, 6, 7, 8, 10}); evalhelper(PredicateExecHelper(pred, Encapsulate(bufactual)) == 1); }
#pragma GCC diagnostic pop

  { uint8_t buf[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }; auto pred = IsEqual(buf); evalhelper(PredicateExecHelper(pred, Encapsulate(bufactual)) == 0); }
  { uint8_t buf[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 10 }; auto pred = IsEqual(buf); evalhelper(PredicateExecHelper(pred, Encapsulate(bufactual)) == 1); }

  { auto pred = IsEqual(std::array<uint8_t, 10>{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 10 }); evalhelper(PredicateExecHelper(pred, Encapsulate(arr)) == 1); }
  { auto pred = IsEqual(std::vector<uint8_t>{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 10 }); evalhelper(PredicateExecHelper(pred, Encapsulate(vec)) == 1); }

  { auto pred = IsEqual(42); evalhelper(PredicateExecHelper(pred, Encapsulate(42)) == 1); }
  { auto pred = IsEqual(42); evalhelper(PredicateExecHelper(pred, Encapsulate(24)) == 0); }

  { auto pred = IsOdd<uint8_t>(); evalhelper(PredicateExecHelper(pred, Encapsulate<uint8_t>(42)) == 0); }
  { auto pred = IsEven<uint8_t>(); evalhelper(PredicateExecHelper(pred, Encapsulate<uint8_t>(21)) == 0); }
  { auto pred = IsDivisibleBy<uint8_t>(7); evalhelper(PredicateExecHelper(pred, Encapsulate<uint8_t>(42)) == 1); }
  { auto pred = IsDivisibleBy<uint8_t>(7); evalhelper(PredicateExecHelper(pred, Encapsulate<uint8_t>(43)) == 0); }
  { auto pred = InBetween<uint8_t>(10, 20); evalhelper(PredicateExecHelper(pred, Encapsulate<uint8_t>(15)) == 1); }
  { auto pred = InBetween<uint8_t>(10, 20); evalhelper(PredicateExecHelper(pred, Encapsulate<uint8_t>(42)) == 0); }
  { auto pred = Outside<uint8_t>(10, 20); evalhelper(PredicateExecHelper(pred, Encapsulate<uint8_t>(42)) == 1); }
  { auto pred = Outside<uint8_t>(10, 20); evalhelper(PredicateExecHelper(pred, Encapsulate<uint8_t>(15)) == 0); }
  { auto pred = IsEqualEpsilon<uint8_t>(15, 2); evalhelper(PredicateExecHelper(pred, Encapsulate<uint8_t>(12)) == 0); }
  { auto pred = IsEqualEpsilon<uint8_t>(15, 2); evalhelper(PredicateExecHelper(pred, Encapsulate<uint8_t>(13)) == 1); }
  { auto pred = IsEqualEpsilon<uint8_t>(15, 2); evalhelper(PredicateExecHelper(pred, Encapsulate<uint8_t>(14)) == 1); }
  { auto pred = IsEqualEpsilon<uint8_t>(15, 2); evalhelper(PredicateExecHelper(pred, Encapsulate<uint8_t>(15)) == 1); }
  { auto pred = IsEqualEpsilon<uint8_t>(15, 2); evalhelper(PredicateExecHelper(pred, Encapsulate<uint8_t>(16)) == 1); }
  { auto pred = IsEqualEpsilon<uint8_t>(15, 2); evalhelper(PredicateExecHelper(pred, Encapsulate<uint8_t>(17)) == 1); }
  { auto pred = IsEqualEpsilon<uint8_t>(15, 2); evalhelper(PredicateExecHelper(pred, Encapsulate<uint8_t>(18)) == 0); }

  { auto pred = IsEqualEpsilon<double>(0.2, 0.001); evalhelper(PredicateExecHelper(pred, Encapsulate(0.3)) == 0); }
  { auto pred = IsEqualEpsilon<double>(0.2, 0.1); evalhelper(PredicateExecHelper(pred, Encapsulate(0.3)) == 1); }
  { auto pred = IsLesserEq<double>(42.5); evalhelper(PredicateExecHelper(pred, Encapsulate(102.3)) == 0); }
  { auto pred = IsLesserEq<double>(42.5); evalhelper(PredicateExecHelper(pred, Encapsulate(12.3)) == 1); }
  { auto pred = IsOdd<double>(); evalhelper(PredicateExecHelper(pred, Encapsulate(102.3)) == 0); }
  { auto pred = IsEven<double>(); evalhelper(PredicateExecHelper(pred, Encapsulate(102.3)) == 0); }
  { auto pred = IsOdd<double>(); evalhelper(PredicateExecHelper(pred, Encapsulate(102.0)) == 0); }
  { auto pred = IsEven<double>(); evalhelper(PredicateExecHelper(pred, Encapsulate(102.0)) == 1); }
  { auto pred = IsOdd<double>(); evalhelper(PredicateExecHelper(pred, Encapsulate(103.0)) == 1); }
  { auto pred = IsEven<double>(); evalhelper(PredicateExecHelper(pred, Encapsulate(103.0)) == 0); }
  { auto pred = IsZero<double>(); evalhelper(PredicateExecHelper(pred, Encapsulate(102.3)) == 0); }
  { auto pred = IsZero<double>(); evalhelper(PredicateExecHelper(pred, Encapsulate(0.0)) == 1); }
  { auto pred = IsNonZero<double>(); evalhelper(PredicateExecHelper(pred, Encapsulate(102.3)) == 1); }
  { auto pred = InBetween<double>(10.2, 10.8); evalhelper(PredicateExecHelper(pred, Encapsulate(10.9)) == 0); }
  { auto pred = IsDivisibleBy<double>(2.5); evalhelper(PredicateExecHelper(pred, Encapsulate(5.0)) == 1); }
  { auto pred = InBetween<uint8_t>(10, 20); evalhelper(PredicateExecHelper(pred, Encapsulate<uint8_t>(42)) == 0); }

  { auto pred = IsEqual<double>(3.14159); evalhelper(PredicateExecHelper(pred, Encapsulate(3.14159)) == 1); }

  ForgetPredicates();

  { auto pred = Obey(testfunc<int>); evalhelper(PredicateExecHelper(pred, Encapsulate(41)) == 0); }
  { auto pred = Obey(testfunc<int>); evalhelper(PredicateExecHelper(pred, Encapsulate(42)) == 1); }
  { auto pred = Obey(testfunc<int>); evalhelper(PredicateExecHelper(pred, Encapsulate(43)) == 0); }

  { auto pred = Resist(testfunc<int>); evalhelper(PredicateExecHelper(pred, Encapsulate(41)) == 1); }
  { auto pred = Resist(testfunc<int>); evalhelper(PredicateExecHelper(pred, Encapsulate(42)) == 0); }
  { auto pred = Resist(testfunc<int>); evalhelper(PredicateExecHelper(pred, Encapsulate(43)) == 1); }

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

  { auto pred = Obey(IsOdd<int>()); evalhelper(PredicateExecHelper(pred, Encapsulate(13)) == 1); }
  { auto pred = Obey(IsOdd<int>()); evalhelper(PredicateExecHelper(pred, Encapsulate(42)) == 0); }
  { auto pred = Resist(IsOdd<int>()); evalhelper(PredicateExecHelper(pred, Encapsulate(13)) == 0); }
  { auto pred = Resist(IsOdd<int>()); evalhelper(PredicateExecHelper(pred, Encapsulate(42)) == 1); }

  // works with lambda
  { auto pred = Obey([](const Encapsulator&){ return true; }); evalhelper(PredicateExecHelper(pred, Encapsulate(13)) == 1); }
  { auto pred = Obey([](const Encapsulator&){ return false; }); evalhelper(PredicateExecHelper(pred, Encapsulate(13)) == 0); }

  // hell yeah, no way to get back the type...
  {
    auto pred = Obey(
      [](const Encapsulator& e)
      {
        const auto& result = Decapsulate<int>(e);
        if (!std::get<0>(result))
        {
          return false;
        }

        const auto& val = std::get<1>(result);
        return val == 42;
      }
    );
    evalhelper(PredicateExecHelper(pred, Encapsulate(42)) == 1);
  }

  {
    auto pred = Obey(
      [](const Encapsulator& e)
      {
        const auto& result = Decapsulate<int>(e);
        if (!std::get<0>(result))
        {
          return false;
        }

        const auto& val = std::get<1>(result);
        return val == 42;
      }
    );
    evalhelper(PredicateExecHelper(pred, Encapsulate(13)) == 0);
  }

  { auto pred = Obey(sumis15<int>); evalhelper(PredicateExecHelper(pred, Encapsulate(twople_t<int>{ 7, 8 })) == 1); }
  { auto pred = Obey(sumis15<int>); evalhelper(PredicateExecHelper(pred, Encapsulate(twople_t<int>{ 8, 7 })) == 1); }
  { auto pred = Obey(sumis15<int>); evalhelper(PredicateExecHelper(pred, Encapsulate(twople_t<int>{ 8, 8 })) == 0); }
  { auto pred = Obey(sumis15<int>); evalhelper(PredicateExecHelper(pred, Encapsulate(twople_t<int>{ 7, 7 })) == 0); }

  // use this as general form to pack any number of parameters of any type
  { auto pred = Obey(sumis15<int>); evalhelper(PredicateExecHelper(pred, Encapsulate(std::make_tuple(7, 8))) == 1); }
  // type checking also works here
  { auto pred = Obey(sumis15<int>); evalhelper(PredicateExecHelper(pred, Encapsulate(std::make_tuple(7U, 8))) == 1); }
  // as well as checking the number of parameters
  { auto pred = Obey(sumis15<int>); evalhelper(PredicateExecHelper(pred, Encapsulate(std::make_tuple(7, 8, 9))) == 1); }

  ForgetPredicates();

  return 0;
}
