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

// hell no, can't play nice to include with the folder
#define UNW_LOCAL_ONLY
#include <libunwind.h>
#undef UNW_LOCAL_ONLY

#include <dlfcn.h>
#include <cxxabi.h>

#include <sstream>

namespace pajko {

namespace tools {

class Unwinder {
public:
  ~Unwinder() noexcept = default;

  static const Unwinder& get() noexcept
  {
    static const Unwinder instance;
    return instance;
  }

  static std::string go() noexcept
  {
    return get().unwind();
  }

private:
  Unwinder() noexcept = default;

  std::string resolve(unw_word_t ip, unw_word_t sp) const noexcept
  {
    std::ostringstream result;
    Dl_info info;

    int status = dladdr(reinterpret_cast<const void*>(ip), &info);
    if (status && info.dli_fname != NULL && info.dli_fname[0] != '\0') {
      if (info.dli_fname) result << info.dli_fname;
      result << "(";

      if (info.dli_sname == NULL) {
        info.dli_saddr = info.dli_fbase;
      }

      if (info.dli_sname || info.dli_saddr) {
        char sign;
        ptrdiff_t offset;

        auto saddr = reinterpret_cast<unw_word_t>(info.dli_saddr);
        if (ip >= saddr) {
          sign = '+';
          offset = ip - saddr;
        } else {
          sign = '-';
          offset = saddr - ip;
        }

        if (info.dli_sname) {
          int status;
          auto sym = info.dli_sname;
          size_t size = 128;

          auto buffer = static_cast<char*>(malloc(size));
          auto demangled = abi::__cxa_demangle(sym, buffer, &size, &status);
          if (demangled) {
            sym = demangled;
            buffer = demangled;
          }
          result << sym;
          free(buffer);
        }

        result << sign << reinterpret_cast<const void*>(offset);
      }
    } else {
      result << "(";
    }
    result << ") [" << reinterpret_cast<const void*>(ip) << "]";
    result << "[" << reinterpret_cast<const void*>(sp) << "]";

    return result.str();
  }

  std::string unwind() const noexcept
  {
    std::ostringstream result;
    unw_context_t uc;
    unw_cursor_t cursor;
    int frame = 1;

    unw_getcontext(&uc);
    unw_init_local(&cursor, &uc);

    while (unw_step(&cursor) > 0) {
      unw_word_t ip, sp;

      unw_get_reg(&cursor, UNW_REG_IP, &ip);
      unw_get_reg(&cursor, UNW_REG_SP, &sp);
      result << std::endl << "Frame #" << frame++ << "\t: ";
      result << resolve(ip, sp);
    }

    return result.str();
  }
};

}; /* namespace tools */

}; /* namespace pajko */
