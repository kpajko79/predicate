#
#  This is free and unencumbered software released into the public domain.
#
#  Anyone is free to copy, modify, publish, use, compile, sell, or
#  distribute this software, either in source code form or as a compiled
#  binary, for any purpose, commercial or non-commercial, and by any
#  means.
#
#  In jurisdictions that recognize copyright laws, the author or authors
#  of this software dedicate any and all copyright interest in the
#  software to the public domain. We make this dedication for the benefit
#  of the public at large and to the detriment of our heirs and
#  successors. We intend this dedication to be an overt act of
#  relinquishment in perpetuity of all present and future rights to this
#  software under copyright law.
#
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
#  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
#  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
#  IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
#  OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
#  ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
#  OTHER DEALINGS IN THE SOFTWARE.
#
#  For more information, please refer to <https://unlicense.org>
#
#
#  SPDX-License-Identifier: Unlicense
#

CPP := gcc
CC := gcc
CXX := g++
CXXLD := g++
CPPCHECK := cppcheck

RM := rm -f
RMDIR := rm -rf
MKDIR := mkdir -p

BUILD_DIR := build

CLANG_STDLIB := libc++

CPPFLAGS := \
	-I. \
	-Iinclude \
	-I/usr/include/libunwind \
	-DPKO_PREDICATE_ENABLE_BACKTRACE \
	-MMD

COMMON_FLAGS := \
	-Og \
	-ggdb3 \
	-fexceptions \
	-funwind-tables \
	-fasynchronous-unwind-tables \
	-fno-dwarf2-cfi-asm \
	-ftrapv \
	-pedantic \
	-Wall \
	-Wextra \
	-Wdouble-promotion \
	-Wformat=2 \
	-Wmisleading-indentation \
	-Wstrict-aliasing \
	-Warray-bounds \
	-Wfloat-equal \
	-Wshadow \
	-Wpedantic \
	-Wcast-qual \
	-Wcast-align \
	-Wconversion \
	-Wno-float-equal \
	$(if $(findstring gcc,$(CXX)),-fvar-tracking-assignments) \
	$(if $(findstring gcc,$(CXX)),-Wformat-overflow=2) \
	$(if $(findstring gcc,$(CXX)),-Wmultistatement-macros) \
	$(if $(findstring gcc,$(CXX)),-Wsuggest-attribute=pure) \
	$(if $(findstring gcc,$(CXX)),-Wsuggest-attribute=const) \
	$(if $(findstring gcc,$(CXX)),-Wsuggest-attribute=malloc) \
	$(if $(findstring gcc,$(CXX)),-Wsuggest-attribute=format) \
	$(if $(findstring gcc,$(CXX)),-Wvla-parameter) \
	$(if $(findstring gcc,$(CXX)),-Warray-parameter) \
	$(if $(findstring gcc,$(CXX)),-Wduplicated-branches) \
	$(if $(findstring gcc,$(CXX)),-Wduplicated-cond) \
	$(if $(findstring gcc,$(CXX)),-Wtrampolines) \
	$(if $(findstring gcc,$(CXX)),-Warith-conversion) \
	$(if $(findstring gcc,$(CXX)),-Wlogical-op)

ASAN_FLAGS := \
	-fsanitize=address \
	-fsanitize=pointer-compare \
	-fsanitize=pointer-subtract \
	-fsanitize=leak

UBSAN_FLAGS := \
	-fsanitize=undefined \
	-fsanitize=shift \
	-fsanitize=shift-exponent \
	-fsanitize=shift-base \
	-fsanitize=integer-divide-by-zero \
	-fsanitize=unreachable \
	-fsanitize=vla-bound \
	-fsanitize=null \
	-fsanitize=return \
	-fsanitize=signed-integer-overflow \
	-fsanitize=bounds \
	-fsanitize=alignment \
	-fsanitize=object-size \
	-fsanitize=float-divide-by-zero \
	-fsanitize=float-cast-overflow \
	-fsanitize=bool \
	-fsanitize=enum \
	-fsanitize=vptr \
	-fsanitize=pointer-overflow \
	$(if $(findstring gcc,$(CXX)),-fsanitize=bounds-strict)

ANALYZER_FLAGS := \
	-fanalyzer \
	-Wanalyzer-too-complex

CFLAGS := \
	-std=c11 \
	$(COMMON_FLAGS)

CXXFLAGS := \
	-std=c++11 \
	$(if $(findstring clang,$(CXX)),-stdlib=$(CLANG_STDLIB)) \
	$(COMMON_FLAGS)

LDFLAGS := \
	$(if $(findstring clang,$(CXX)),-stdlib=$(CLANG_STDLIB)) \
	-rdynamic

LDLIBS := -lunwind

backslash := \\
openingbrace := (
closingbrace := )
quote := \"

CINCLUDES := $(addprefix -I, $(shell $(CC) $(CFLAGS) -E -Wp,-v -xc /dev/null 2> /dev/stdout | grep -e "^ "))

CXXINCLUDES := $(addprefix -I, $(shell $(CXX) $(CXXFLAGS) -E -Wp,-v -xc++ /dev/null 2> /dev/stdout | grep -e "^ "))

CMACROS := $(shell $(CC) $(CFLAGS) -E -dM -xc /dev/null 2> /dev/stdout | \
	sed -E "s/#define ([^ ]*) (.*)/'-D\1=\2'/g;")

CXXMACROS := $(shell $(CXX) $(CXXFLAGS) -E -dM -xc++ /dev/null 2> /dev/stdout | \
	sed -E "s/#define ([^ ]*) (.*)/'-D\1=\2'/g;")

# cool, cppcheck reports an internalAstError:
# /usr/include/c++/12/bits/stl_iterator.h:201:7: error: Syntax Error: AST broken, '__x' doesn't have a parent.
# and it reports a syntaxError with libc++:
# /usr/include/c++/v1/tuple:612:12: error: syntax error [syntaxError]
# so don't add $(CXXINCLUDES) for now
CPPCHECK_FLAGS := \
	$(CXXMACROS) \
	-D__extension__= \
	-U_LIBCPP_HAS_NO_ATOMIC_HEADER \
	-UNDEBUG \
	-UPKO_STREAM_ARRAY_LIMIT \
	-UPKO_PREDICATE_DO_DEBUGBREAK \
	-I. \
	-Iinclude \
	-I/usr/include/libunwind \
	-Ijunk \
	--platform=unix64 \
	--language=c++ \
	--std=c++11 \
	--enable=all \
	--inline-suppr \
	--report-progress

all: $(BUILD_DIR)/test_predicate $(BUILD_DIR)/test_predicate_dbghook

test: address_sanitizer undefined_sanitizer analyzer

address_sanitizer: $(BUILD_DIR)/test_predicate_asan
	$(addprefix ./,$<)

undefined_sanitizer: $(BUILD_DIR)/test_predicate_ubsan
	$(addprefix ./,$<)

analyzer: $(BUILD_DIR)/test_predicate_analyzer
	$(addprefix ./,$<)

$(BUILD_DIR)/%: $(BUILD_DIR)/%.o
	$(CXXLD) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(BUILD_DIR)/test_predicate_asan: LDFLAGS += $(ASAN_FLAGS)
$(BUILD_DIR)/test_predicate_ubsan: LDFLAGS += $(UBSAN_FLAGS)
$(BUILD_DIR)/test_predicate_analyzer: LDFLAGS += $(ANALYZER_FLAGS)

$(BUILD_DIR)/%_asan.o: CXXFLAGS += $(ASAN_FLAGS)
$(BUILD_DIR)/%_ubsan.o: CXXFLAGS += $(UBSAN_FLAGS)
$(BUILD_DIR)/%_analyzer.o: CXXFLAGS += $(ANALYZER_FLAGS)
$(BUILD_DIR)/%_dbghook.o: CXXFLAGS += -DPKO_PREDICATE_DO_DEBUGBREAK

$(BUILD_DIR)/%_asan.o: %.cpp | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

$(BUILD_DIR)/%_ubsan.o: %.cpp | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

$(BUILD_DIR)/%_analyzer.o: %.cpp | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

$(BUILD_DIR)/%_dbghook.o: %.cpp | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: %.cpp | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

clean:
	$(RMDIR) $(BUILD_DIR)

$(BUILD_DIR):
	$(MKDIR) $@

cppcheck: test_predicate.cpp
	$(CPPCHECK) $(CPPCHECK_FLAGS) $<

.PHONY: clean cppcheck

-include $(BUILD_DIR)/*.d
