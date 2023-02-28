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

#include <sstream>
#include <type_traits>

namespace pajko {

namespace handicap {

template<class CharT, class Traits = std::char_traits<CharT>, class Allocator = std::allocator<CharT>>
class handicap_ostringstream
{
private:
  using super = std::basic_ostringstream<CharT, Traits, Allocator>;

public:
  handicap_ostringstream() noexcept
    : stream(std::ios_base::out)
  {}

  explicit handicap_ostringstream(std::ios_base::openmode mode) noexcept
    : stream(mode)
  {}

  explicit handicap_ostringstream(const std::basic_string<CharT, Traits, Allocator>& str,
    std::ios_base::openmode mode = std::ios_base::out) noexcept
    : stream(str, mode)
  {}

  handicap_ostringstream(handicap_ostringstream&& other) noexcept
   : stream(std::move(other.stream))
  {}

  handicap_ostringstream(super&& other) noexcept
   : stream(std::move(other))
  {}

  handicap_ostringstream& operator=(handicap_ostringstream&& other) noexcept
  {
    stream = std::move(other.stream);
    return *this;
  }

  handicap_ostringstream& operator=(super&& other) noexcept
  {
    stream = std::move(other);
    return *this;
  }

  void swap(handicap_ostringstream& other) noexcept
  {
    stream.swap(other.stream);
  }

  void swap(super& other) noexcept
  {
    stream.swap(other);
  }

  std::basic_stringbuf<CharT, Traits, Allocator>* rdbuf() const noexcept
  {
    return stream.rdbuf();
  }

  std::basic_string<CharT, Traits, Allocator> str() const noexcept
  {
    return stream.str();
  }

  handicap_ostringstream& put(typename super::char_type ch) noexcept
  {
    stream.put(ch);
    return *this;
  }

  handicap_ostringstream& write(const typename super::char_type* s, std::streamsize count) noexcept
  {
    stream.write(s, count);
    return *this;
  }

  typename super::pos_type tellp() noexcept
  {
    return stream.tellp();
  }

  handicap_ostringstream& seekp(typename super::pos_type pos) noexcept
  {
    stream.seekp(pos);
    return *this;
  }

  handicap_ostringstream& seekp(typename super::off_type off, std::ios_base::seekdir dir) noexcept
  {
    stream.seekp(off, dir);
    return *this;
  }

  handicap_ostringstream& flush() noexcept
  {
    stream.flush();
    return *this;
  }

  template <typename T>
  auto operator<<(T&& c) noexcept ->
  typename std::enable_if<
    std::is_signed<typename std::decay<T>::type>::value &&
    (std::is_same<typename std::decay<T>::type, char>::value ||
     std::is_same<typename std::decay<T>::type, wchar_t>::value),
    handicap_ostringstream&
  >::type
  {
    stream << std::move(static_cast<signed int>(c));
    return *this;
  }

  template <typename T>
  auto operator<<(T&& c) noexcept ->
  typename std::enable_if<
    std::is_unsigned<typename std::decay<T>::type>::value &&
    (std::is_same<typename std::decay<T>::type, char>::value ||
     std::is_same<typename std::decay<T>::type, wchar_t>::value ||
     std::is_same<typename std::decay<T>::type, unsigned char>::value ||
     std::is_same<typename std::decay<T>::type, char16_t>::value ||
     std::is_same<typename std::decay<T>::type, char32_t>::value),
    handicap_ostringstream&
  >::type
  {
    stream << std::move(static_cast<unsigned int>(c));
    return *this;
  }

  template <typename T>
  auto operator<<(T&& c) noexcept ->
  typename std::enable_if<
    !std::is_same<typename std::decay<T>::type, char>::value &&
    !std::is_same<typename std::decay<T>::type, unsigned char>::value &&
    !std::is_same<typename std::decay<T>::type, wchar_t>::value &&
    !std::is_same<typename std::decay<T>::type, char16_t>::value &&
    !std::is_same<typename std::decay<T>::type, char32_t>::value,
    handicap_ostringstream&
  >::type
  {
    stream << std::move(c);
    return *this;
  }

private:
  super stream;
};

using ostringstream = handicap_ostringstream<char>;
using wostringstream = handicap_ostringstream<wchar_t>;

template<class CharT, class Traits, class Alloc>
inline void swap(handicap_ostringstream<CharT, Traits, Alloc>& lhs, handicap_ostringstream<CharT, Traits, Alloc>& rhs) noexcept
{
  lhs.swap(rhs);
}

template<class CharT, class Traits, class Alloc>
inline void swap(handicap_ostringstream<CharT, Traits, Alloc>& lhs, std::basic_ostringstream<CharT, Traits, Alloc>& rhs) noexcept
{
  lhs.swap(rhs);
}

template<class CharT, class Traits, class Alloc>
inline void swap(std::basic_ostringstream<CharT, Traits, Alloc>& lhs, handicap_ostringstream<CharT, Traits, Alloc>& rhs) noexcept
{
  lhs.swap(rhs);
}

}; /* namespace handicap */

}; /* namespace pajko */
