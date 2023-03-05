/*
 * MIT License
 *
 * Copyright (c) 2023, Patrik Kluba <kpajko79@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include <memory>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <type_traits>
#include <typeinfo>
#include <vector>
#include <cmath>

#include "integer_sequence.hpp"
#include "handicap_ostringstream.hpp"
#include "compiler_support.hpp"

#if defined(PKO_PREDICATE_ENABLE_BACKTRACE) && !defined(PKO_PREDICATE_DEBUGBREAK)
#include "unwind_tool.hpp"
#endif

namespace pajko {

namespace Predicate {

#if defined(NDEBUG)
#define PKO_PREDICATE_DEBUGBREAK                                        \
  std::abort()
#else
#define PKO_PREDICATE_DEBUGBREAK                                        \
  assert(false)
#endif

#if defined(PKO_PREDICATE_DO_DEBUGBREAK)
#define PKO_PREDICATE_UNWIND_HOOK(msg)                                  \
  PKO_PREDICATE_DEBUGBREAK
#elif !defined(PKO_PREDICATE_ENABLE_BACKTRACE)
#define PKO_PREDICATE_UNWIND_HOOK(msg)
#else
#define PKO_PREDICATE_UNWIND_HOOK(msg)                                  \
  msg << ::pajko::tools::Unwinder::go()
#endif

#if !defined(PKO_PREDICATE_LOGGER_HELPER)
#define PKO_PREDICATE_LOGGER(result, text)                              \
[&]() noexcept { return result; }()
#else
#define PKO_PREDICATE_LOGGER(result, text)                              \
[&]() noexcept {                                                        \
  auto _result = (result);                                              \
  if (!_result) {                                                       \
    handicap::ostringstream msg;                                        \
    msg << std::endl;                                                   \
    msg << text;                                                        \
    PKO_PREDICATE_UNWIND_HOOK(msg);                                     \
    PKO_PREDICATE_LOGGER_HELPER(_result, msg.str().c_str());            \
  }                                                                     \
  return _result;                                                       \
}()
#endif

class Encapsulator
{
public:
  virtual ~Encapsulator() noexcept = default;
  virtual const void* fetch() const noexcept = 0;

  operator const void*() const noexcept {
    return this;
  }

  virtual size_t hash_code() const noexcept = 0;
  virtual const char* type_name() const noexcept = 0;

protected:
  Encapsulator() noexcept = default;
};

class Predicate
{
public:
  virtual ~Predicate() noexcept = default;
  virtual bool execute(const Encapsulator&) const noexcept = 0;

protected:
  Predicate() noexcept = default;
};

namespace impl {

static std::vector<std::unique_ptr<Predicate>> holder;

template <typename T>
class EncapsulatorImpl : public Encapsulator
{
public:
  EncapsulatorImpl() noexcept = delete;
  ~EncapsulatorImpl() noexcept = default;

  EncapsulatorImpl(const T& what) noexcept
    : _what(what)
    , _ti(typeid(T))
  {}

  const void* fetch() const noexcept override
  {
    return &_what;
  }

  size_t hash_code() const noexcept override
  {
    return _ti.hash_code();
  }

  const char* type_name() const noexcept override
  {
    return _ti.name();
  }

private:
  const T _what;
  const std::type_info& _ti;
};

template <typename T, size_t N>
using array_wrapper = std::tuple<const T(&)[N]>;

}; /* namespace impl */

template <typename T, size_t N>
inline
impl::EncapsulatorImpl<impl::array_wrapper<T, N>> Encapsulate(const T (&arg)[N]) noexcept
{
  return impl::array_wrapper<T, N>{ arg };
}

template <typename T, size_t N>
inline
impl::EncapsulatorImpl<impl::array_wrapper<T, N>> Encapsulate(T (&&arg)[N]) noexcept
{
  return impl::array_wrapper<T, N>{ std::cref(arg) };
}

template <typename T>
inline
auto Encapsulate(const T& what) noexcept ->
  typename std::enable_if<
    !std::is_pointer<typename std::decay<T>::type>::value,
    impl::EncapsulatorImpl<T>
  >::type
{
  return what;
}

template <typename T, size_t N>
inline std::tuple<bool, const T(&)[N]> Decapsulate(const Encapsulator& what) noexcept
{
  const std::type_info& ti = typeid(impl::array_wrapper<T, N>);

  auto result = PKO_PREDICATE_LOGGER(ti.hash_code() == what.hash_code(),
      "The actual value has type '"
      << ::pajko::tools::Unwinder::decodeTypeName(what.type_name())
      << "' while the expected was '"
      << ::pajko::tools::Unwinder::decodeTypeName(ti.name())
      << "'"
  );

  return { result, std::get<0>(*reinterpret_cast<const impl::array_wrapper<T, N>*>(what.fetch())) };
}

template <typename T>
inline std::tuple<bool, const T&> Decapsulate(const Encapsulator& what) noexcept
{
  const std::type_info& ti = typeid(T);

  auto result = PKO_PREDICATE_LOGGER(ti.hash_code() == what.hash_code(),
      "The actual value has type '"
      << ::pajko::tools::Unwinder::decodeTypeName(what.type_name())
      << "' while the expected was '"
      << ::pajko::tools::Unwinder::decodeTypeName(ti.name())
      << "'"
  );

  return { result, *(reinterpret_cast<const T*>(what.fetch())) };
}

inline void ForgetPredicates() noexcept
{
  impl::holder.clear();
}

namespace impl {

template<typename... T>
class WithArgsImpl : public Predicate
{
private:
  using func_prototype_t = std::function<bool(const Encapsulator& arg, T...)>;

public:
  static const void* create(func_prototype_t&& func, T&&... args) noexcept
  {
    auto instance = std::unique_ptr<WithArgsImpl<T...>>(new WithArgsImpl<T...>(std::move(func), std::forward<T>(args)...));
    auto instancePtr = instance.get();
    holder.emplace_back(std::move(instance));
    return instancePtr;
  }

  static const void* create(bool(*func)(const Encapsulator& arg, T...), T&&... args) noexcept
  {
    return create(func_prototype_t(func), std::forward<T>(args)...);
  }

  bool execute(const Encapsulator& what) const noexcept override
  {
    return executeHelper(what,
    detail::make_index_sequence<
      std::tuple_size<decltype(_args)>::value
    >{});
  }

  ~WithArgsImpl() noexcept override = default;

private:
  template <std::size_t... Is>
  inline bool executeHelper(const Encapsulator& what, detail::index_sequence<Is...>) const noexcept
  {
    return _func(what, std::get<Is>(_args)...);
  }

  WithArgsImpl(func_prototype_t&& func, T&&... args) noexcept
    : _func(std::move(func))
    , _args(std::forward<T>(args)...)
  {}

  const func_prototype_t _func;
  const std::tuple<T...> _args;
};

}; /* namespace impl */

template <typename... T>
inline const void* WithArgs(std::function<bool(const Encapsulator& arg, T...)>&& func, T&&... args) noexcept
{
  return impl::WithArgsImpl<T...>::create(std::move(func), std::forward<T>(args)...);
}

template <typename... T>
inline const void* WithArgs(bool(*func)(const Encapsulator& arg, T...), T&&... args) noexcept
{
  return impl::WithArgsImpl<T...>::create(func, std::forward<T>(args)...);
}

#define PKO_PREDICATE_EVAL_GENERATOR(name, val)                                                               \
namespace impl {                                                                                              \
                                                                                                              \
template <typename T>                                                                                         \
class name ## Impl : public Predicate                                                                         \
{                                                                                                             \
public:                                                                                                       \
  static const void* create(T&& func) noexcept                                                                \
  {                                                                                                           \
    auto instance = std::unique_ptr<name ## Impl<T>>(new name ## Impl<T>(std::move(func)));                   \
    auto instancePtr = instance.get();                                                                        \
    holder.emplace_back(std::move(instance));                                                                 \
    return instancePtr;                                                                                       \
  }                                                                                                           \
                                                                                                              \
  bool execute(const Encapsulator& what) const noexcept override                                              \
  {                                                                                                           \
    return executeHelper(what) == val;                                                                        \
  }                                                                                                           \
                                                                                                              \
  ~name ## Impl() noexcept override = default;                                                                \
                                                                                                              \
private:                                                                                                      \
  template <typename U = T>                                                                                   \
  inline auto executeHelper(const Encapsulator& what) const noexcept ->                                       \
    typename std::enable_if<                                                                                  \
      std::is_same<U, const void*>::value,                                                                    \
      bool                                                                                                    \
    >::type                                                                                                   \
  {                                                                                                           \
    auto predicate = reinterpret_cast<const Predicate*>(_func);                                               \
    return predicate->execute(what);                                                                          \
  }                                                                                                           \
                                                                                                              \
  template <typename U = T>                                                                                   \
  inline auto executeHelper(const Encapsulator& what) const noexcept ->                                       \
    typename std::enable_if<                                                                                  \
      !std::is_same<U, const void*>::value,                                                                   \
      bool                                                                                                    \
    >::type                                                                                                   \
  {                                                                                                           \
    return _func(what);                                                                                       \
  }                                                                                                           \
                                                                                                              \
  name ## Impl(T&& func) noexcept                                                                             \
    : _func(std::move(func))                                                                                  \
  {}                                                                                                          \
                                                                                                              \
  const T _func;                                                                                              \
};                                                                                                            \
                                                                                                              \
}; /* namespace impl */                                                                                       \
                                                                                                              \
template <typename T>                                                                                         \
inline const void* name(T&& func) noexcept                                                                    \
{                                                                                                             \
  return impl::name ## Impl<T>::create(std::move(func));                                                      \
}

PKO_PREDICATE_EVAL_GENERATOR(Obey, true)
PKO_PREDICATE_EVAL_GENERATOR(Resist, false)

#undef PKO_PREDICATE_EVAL_GENERATOR

#define PKO_PREDICATE_MATCHER_GENERATOR(name)                                                                                  \
namespace impl {                                                                                                               \
                                                                                                                               \
template<typename... T>                                                                                                        \
class Match ## name ## Impl : public Predicate                                                                                 \
{                                                                                                                              \
public:                                                                                                                        \
  static const void* create(T&&... funcs) noexcept                                                                             \
  {                                                                                                                            \
    auto instance = std::unique_ptr<Match ## name ## Impl<T...>>(new Match ## name ## Impl<T...>(std::forward<T>(funcs)...));  \
    auto instancePtr = instance.get();                                                                                         \
    holder.emplace_back(std::move(instance));                                                                                  \
    return instancePtr;                                                                                                        \
  }                                                                                                                            \
                                                                                                                               \
  bool execute(const Encapsulator& what) const noexcept override                                                               \
  {                                                                                                                            \
    return executeHelper(what,                                                                                                 \
    detail::make_index_sequence<                                                                                               \
      std::tuple_size<decltype(_funcs)>::value                                                                                 \
    >{});                                                                                                                      \
  }                                                                                                                            \
                                                                                                                               \
  ~Match ## name ## Impl() noexcept override = default;                                                                        \
                                                                                                                               \
private:                                                                                                                       \
  inline bool executeHelper2(bool (*func)(const Encapsulator&), const Encapsulator& what) const noexcept                       \
  {                                                                                                                            \
    return func(what);                                                                                                         \
  }                                                                                                                            \
                                                                                                                               \
  inline bool executeHelper2(std::function<bool(const Encapsulator&)>&& func,                                                  \
    const Encapsulator& what) const noexcept                                                                                   \
  {                                                                                                                            \
    return func(what);                                                                                                         \
  }                                                                                                                            \
                                                                                                                               \
  inline bool executeHelper2(const void* func, const Encapsulator& what) const noexcept                                        \
  {                                                                                                                            \
    auto predicate = reinterpret_cast<const Predicate*>(func);                                                                 \
    return predicate->execute(what);                                                                                           \
  }                                                                                                                            \
                                                                                                                               \
  template <std::size_t... Is>                                                                                                 \
  inline bool executeHelper(const Encapsulator& what, detail::index_sequence<Is...>) const noexcept                            \
  {                                                                                                                            \
    std::vector<bool> results{ (executeHelper2(std::get<Is>(_funcs), what))... };                                              \
    return helper::MatcherHelper ## name(std::move(results));                                                                  \
  }                                                                                                                            \
                                                                                                                               \
  Match ## name ## Impl(T&&... funcs) noexcept                                                                                 \
    : _funcs(std::forward<T>(funcs)...)                                                                                        \
  {}                                                                                                                           \
                                                                                                                               \
  const std::tuple<T...> _funcs;                                                                                               \
};                                                                                                                             \
                                                                                                                               \
}; /* namespace impl */                                                                                                        \
                                                                                                                               \
template<typename... T>                                                                                                        \
inline const void* Match ## name(T&&... funcs) noexcept                                                                        \
{                                                                                                                              \
  return impl::Match ## name ## Impl<T...>::create(std::forward<T>(funcs)...);                                                 \
}

namespace impl {

namespace helper {

PKO_PURE inline bool MatcherHelperAll(std::vector<bool>&& results) noexcept
{
  bool result = true;

  for (auto r : results) {
    result &= r;
  }

  return result;
}

PKO_PURE inline bool MatcherHelperNone(std::vector<bool>&& results) noexcept
{
  bool result = false;

  for (auto r : results) {
    result |= r;
  }

  return !result;
}

PKO_PURE inline bool MatcherHelperAny(std::vector<bool>&& results) noexcept
{
  for (auto r : results) {
    if (r) {
      return true;
    }
  }

  return false;
}

PKO_PURE inline bool MatcherHelperOne(std::vector<bool>&& results) noexcept
{
  bool result = false;

  for (auto r : results) {
    if (r) {
      if (!result) {
        result = true;
      } else {
        return false;
      }
    }
  }

  return true;
}

}; /* namespace helper */

}; /* namespace impl */

PKO_PREDICATE_MATCHER_GENERATOR(All)
PKO_PREDICATE_MATCHER_GENERATOR(Any)
PKO_PREDICATE_MATCHER_GENERATOR(One)
PKO_PREDICATE_MATCHER_GENERATOR(None)

#undef PKO_PREDICATE_MATCHER_GENERATOR

namespace impl {

template <typename T, size_t N>
class IsEqualImpl : public Predicate
{
public:
  static const void* checker(const T (&arg)[N]) noexcept
  {
    auto instance = std::unique_ptr<IsEqualImpl<T, N>>(new IsEqualImpl<T, N>(arg));
    auto instancePtr = instance.get();
    holder.emplace_back(std::move(instance));
    return instancePtr;
  }

  static const void* checker(T (&&arg)[N]) noexcept
  {
    auto instance = std::unique_ptr<IsEqualImpl<T, N>>(new IsEqualImpl<T, N>(std::move(arg)));
    auto instancePtr = instance.get();
    holder.emplace_back(std::move(instance));
    return instancePtr;
  }

  bool execute(const Encapsulator& what) const noexcept override
  {
    const auto& result = Decapsulate<T, N>(what);
    if (!std::get<0>(result))
    {
      return false;
    }

    const auto& val = std::get<1>(result);
    return PKO_PREDICATE_LOGGER(
      std::memcmp(_arg, &val, N * sizeof(T)) == 0,
      "Predicate IsEqual(" << _arg << ") failed for value " << val
    );
  }

  ~IsEqualImpl() noexcept override = default;

private:
  IsEqualImpl(const T (&arg)[N]) noexcept
  {
    std::memcpy(_arg, arg, N * sizeof(T));
  }

  IsEqualImpl(T (&&arg)[N]) noexcept
  {
    std::memcpy(_arg, arg, N * sizeof(T));
  }

  T _arg[N];
};

template <typename T>
class IsEqualImpl<T, 0> : public Predicate
{
public:
  IsEqualImpl() noexcept = delete;

  static const void* checker(T&& arg) noexcept
  {
    auto instance = std::unique_ptr<IsEqualImpl<T, 0>>(new IsEqualImpl<T, 0>(std::move(arg)));
    auto instancePtr = instance.get();
    holder.emplace_back(std::move(instance));
    return instancePtr;
  }

  bool execute(const Encapsulator& what) const noexcept override
  {
    const auto& result = Decapsulate<T>(what);
    if (!std::get<0>(result))
    {
      return false;
    }

    const auto& val = std::get<1>(result);
    return PKO_PREDICATE_LOGGER(val == _arg, "Predicate IsEqual(" << _arg << ") failed for value " << val);
  }

  ~IsEqualImpl() noexcept override = default;

private:
  IsEqualImpl(T&& arg) noexcept
    : _arg(std::move(arg))
  {}

  const T _arg;
};

}; /* namespace impl */

template <typename T, size_t N>
inline
const void* IsEqual(const T (&arg)[N]) noexcept
{
  return impl::IsEqualImpl<T, N>::checker(arg);
}

template <typename T, size_t N>
inline
const void* IsEqual(T (&&arg)[N]) noexcept
{
  return impl::IsEqualImpl<T, N>::checker(std::move(arg));
}

template <typename T>
inline
auto IsEqual(T&& param) noexcept ->
  typename std::enable_if<
    !std::is_pointer<typename std::decay<T>::type>::value,
    const void*
  >::type
{
  return impl::IsEqualImpl<T, 0>::checker(std::move(param));
}

#define PKO_PREDICATE_FILTER_GENERATOR(name, filter)                                            \
namespace impl {                                                                                \
                                                                                                \
template <typename T>                                                                           \
class name ## Impl : public Predicate                                                           \
{                                                                                               \
public:                                                                                         \
  static const void* checker() noexcept                                                         \
  {                                                                                             \
    static auto instance = std::unique_ptr<name ## Impl<T>>(new name ## Impl<T>());             \
    return instance.get();                                                                      \
  }                                                                                             \
                                                                                                \
  bool execute(const Encapsulator& what) const noexcept override                                \
  {                                                                                             \
    const auto& result = Decapsulate<T>(what);                                                  \
    if (!std::get<0>(result))                                                                   \
    {                                                                                           \
      return false;                                                                             \
    }                                                                                           \
                                                                                                \
    const auto& val = std::get<1>(result);                                                      \
    return PKO_PREDICATE_LOGGER(                                                                \
      [](T arg) noexcept {filter}(val),                                                         \
      "Predicate " #name "() failed for value " << val                                          \
    );                                                                                          \
  }                                                                                             \
                                                                                                \
  ~name ## Impl() noexcept override = default;                                                  \
                                                                                                \
private:                                                                                        \
  name ## Impl() noexcept                                                                       \
  {}                                                                                            \
};                                                                                              \
                                                                                                \
}; /* namespace impl */                                                                         \
                                                                                                \
template <typename T>                                                                           \
inline                                                                                          \
const void* name() noexcept                                                                     \
{                                                                                               \
  return impl::name ## Impl<T>::checker();                                                      \
}

namespace impl {

namespace helper {

template <typename T>
inline auto IsOddHelper(T&& arg) noexcept ->
  typename std::enable_if<
    std::is_integral<T>::value, bool
  >::type
{
  return (arg % 2) == 1;
}

template <typename T>
inline auto IsOddHelper(T&& arg) noexcept ->
  typename std::enable_if<
    std::is_same<T, double>::value, bool
  >::type
{
  // yuck
  return fmod(arg, 2.0) == 1.0;
}

template <typename T>
inline auto IsOddHelper(T&& arg) noexcept ->
  typename std::enable_if<
    std::is_same<T, long double>::value, bool
  >::type
{
  // yuck
  return fmodl(arg, 2.0) == 1.0;
}

template <typename T>
inline auto IsOddHelper(T&& arg) noexcept ->
  typename std::enable_if<
    std::is_same<T, float>::value, bool
  >::type
{
  // yuck
  return fmodf(arg, 2.0f) == 1.0f;
}

template <typename T>
inline auto IsEvenHelper(T&& arg) noexcept ->
  typename std::enable_if<
    std::is_integral<T>::value, bool
  >::type
{
  return (arg % 2) == 0;
}

template <typename T>
inline auto IsEvenHelper(T&& arg) noexcept ->
  typename std::enable_if<
    std::is_same<T, double>::value, bool
  >::type
{
  // yuck
  return fmod(arg, 2.0) == 0.0;
}

template <typename T>
inline auto IsEvenHelper(T&& arg) noexcept ->
  typename std::enable_if<
    std::is_same<T, long double>::value, bool
  >::type
{
  // yuck
  return fmodl(arg, 2.0) == 0.0;
}

template <typename T>
inline auto IsEvenHelper(T&& arg) noexcept ->
  typename std::enable_if<
    std::is_same<T, float>::value, bool
  >::type
{
  // yuck
  return fmodf(arg, 2.0f) == 0.0f;
}

}; /* namespace helper */

}; /* namespace impl */

// auto in lambda would be C++14
PKO_PREDICATE_FILTER_GENERATOR(IsOdd, return helper::IsOddHelper(std::move(arg));)
PKO_PREDICATE_FILTER_GENERATOR(IsEven, return helper::IsEvenHelper(std::move(arg));)
PKO_PREDICATE_FILTER_GENERATOR(IsZero, return arg == 0;)
PKO_PREDICATE_FILTER_GENERATOR(IsNonZero, return arg != 0;)
PKO_PREDICATE_FILTER_GENERATOR(IsPositive, return arg > 0;)
PKO_PREDICATE_FILTER_GENERATOR(IsNegative, return arg < 0;)

#undef PKO_PREDICATE_FILTER_GENERATOR

#define PKO_PREDICATE_FILTER_GENERATOR_PARAM(name, filter)                                      \
namespace impl {                                                                                \
                                                                                                \
template <typename T>                                                                           \
class name ## Impl : public Predicate                                                           \
{                                                                                               \
public:                                                                                         \
  name ## Impl() noexcept = delete;                                                             \
                                                                                                \
  static const void* checker(T&& param) noexcept                                                \
  {                                                                                             \
    auto instance = std::unique_ptr<name ## Impl<T>>(new name ## Impl<T>(std::move(param)));    \
    auto instancePtr = instance.get();                                                          \
    holder.emplace_back(std::move(instance));                                                   \
    return instancePtr;                                                                         \
  }                                                                                             \
                                                                                                \
  bool execute(const Encapsulator& what) const noexcept override                                \
  {                                                                                             \
    const auto& result = Decapsulate<T>(what);                                                  \
    if (!std::get<0>(result))                                                                   \
    {                                                                                           \
      return false;                                                                             \
    }                                                                                           \
                                                                                                \
    const auto& val = std::get<1>(result);                                                      \
    return PKO_PREDICATE_LOGGER(                                                                \
      [](T arg, T param) noexcept {filter}(val, _param),                                        \
      "Predicate " #name "(" << _param << ") failed for value " << val                          \
    );                                                                                          \
  }                                                                                             \
                                                                                                \
  ~name ## Impl() noexcept override = default;                                                  \
                                                                                                \
private:                                                                                        \
  name ## Impl(T&& param) noexcept                                                              \
    : _param(std::move(param))                                                                  \
  {}                                                                                            \
                                                                                                \
  const T _param;                                                                               \
};                                                                                              \
                                                                                                \
}; /* namespace impl */                                                                         \
                                                                                                \
template <typename T>                                                                           \
inline                                                                                          \
const void* name(T&& param) noexcept                                                            \
{                                                                                               \
  return impl::name ## Impl<T>::checker(std::move(param));                                      \
}

namespace impl {

namespace helper {

template <typename T>
inline auto IsDivisibleByHelper(T&& arg, T&& param) noexcept ->
  typename std::enable_if<
    std::is_integral<T>::value, bool
  >::type
{
  return (arg % param) == 0;
}

template <typename T>
inline auto IsDivisibleByHelper(T&& arg, T&& param) noexcept ->
  typename std::enable_if<
    std::is_same<T, double>::value, bool
  >::type
{
  // yuck
  return fmod(arg, param) == 0.0;
}

template <typename T>
inline auto IsDivisibleByHelper(T&& arg, T&& param) noexcept ->
  typename std::enable_if<
    std::is_same<T, long double>::value, bool
  >::type
{
  // yuck
  return fmodl(arg, param) == 0.0;
}

template <typename T>
inline auto IsDivisibleByHelper(T&& arg, T&& param) noexcept ->
  typename std::enable_if<
    std::is_same<T, float>::value, bool
  >::type
{
  // yuck
  return fmodf(arg, param) == 0.0f;
}

}; /* namespace helper */

}; /* namespace impl */

// auto in lambda would be C++14
PKO_PREDICATE_FILTER_GENERATOR_PARAM(IsDivisibleBy, return helper::IsDivisibleByHelper(std::move(arg), std::move(param));)
PKO_PREDICATE_FILTER_GENERATOR_PARAM(IsLesserThan, return arg < param;)
PKO_PREDICATE_FILTER_GENERATOR_PARAM(IsLesserEq, return arg <= param;)
PKO_PREDICATE_FILTER_GENERATOR_PARAM(IsGreaterThan, return arg > param;)
PKO_PREDICATE_FILTER_GENERATOR_PARAM(IsGreaterEq, return arg >= param;)

#undef PKO_PREDICATE_FILTER_GENERATOR_PARAM

#define PKO_PREDICATE_FILTER_GENERATOR_2PARAM(name, filter)                                              \
namespace impl {                                                                                         \
                                                                                                         \
template <typename T>                                                                                    \
class name ## Impl : public Predicate                                                                    \
{                                                                                                        \
public:                                                                                                  \
  name ## Impl() noexcept = delete;                                                                      \
                                                                                                         \
  static const void* checker(T&& p1, T&& p2) noexcept                                                    \
  {                                                                                                      \
    auto instance = std::unique_ptr<name ## Impl<T>>(new name ## Impl<T>(std::move(p1), std::move(p2))); \
    auto instancePtr = instance.get();                                                                   \
    holder.emplace_back(std::move(instance));                                                            \
    return instancePtr;                                                                                  \
  }                                                                                                      \
                                                                                                         \
  bool execute(const Encapsulator& what) const noexcept override                                         \
  {                                                                                                      \
    const auto& result = Decapsulate<T>(what);                                                           \
    if (!std::get<0>(result))                                                                            \
    {                                                                                                    \
      return false;                                                                                      \
    }                                                                                                    \
                                                                                                         \
    const auto& val = std::get<1>(result);                                                               \
    return PKO_PREDICATE_LOGGER(                                                                         \
      [](T arg, T param1, T param2) noexcept {filter}(val, _p1, _p2),                                    \
      "Predicate " #name "(" << _p1 << ", " << _p2 << ") failed for value " << val                       \
    );                                                                                                   \
  }                                                                                                      \
                                                                                                         \
  ~name ## Impl() noexcept override = default;                                                           \
                                                                                                         \
private:                                                                                                 \
  name ## Impl(T&& p1, T&& p2) noexcept                                                                  \
    : _p1(std::move(p1))                                                                                 \
    , _p2(std::move(p2))                                                                                 \
  {}                                                                                                     \
                                                                                                         \
  const T _p1;                                                                                           \
  const T _p2;                                                                                           \
};                                                                                                       \
                                                                                                         \
}; /* namespace impl */                                                                                  \
                                                                                                         \
template <typename T>                                                                                    \
inline                                                                                                   \
const void* name(T&& p1, T&& p2) noexcept                                                                \
{                                                                                                        \
  return impl::name ## Impl<T>::checker(std::move(p1), std::move(p2));                                   \
}

PKO_PREDICATE_FILTER_GENERATOR_2PARAM(InBetween, return arg >= param1 && arg <= param2;)
PKO_PREDICATE_FILTER_GENERATOR_2PARAM(Outside, return arg < param1 || arg > param2;)
PKO_PREDICATE_FILTER_GENERATOR_2PARAM(IsEqualEpsilon, return (arg >= param1) ? (arg - param1) <= param2 : (param1 - arg) <= param2;)

#undef PKO_PREDICATE_FILTER_GENERATOR_2PARAM

bool PredicateExecHelper(const void* expected, const void* actual) noexcept
{
  auto predicate = reinterpret_cast<const Predicate*>(expected);
  return predicate->execute(*reinterpret_cast<const Encapsulator*>(actual));
}

}; /* namespace Predicate */

}; /* namespace pajko */

// leave PKO_PREDICATE_LOGGER macro available for external use
// PKO_PREDICATE_UNWIND_HOOK is required as well
