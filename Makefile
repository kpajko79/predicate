CPP := gcc
CC := gcc
CXX := g++
CXXLD := g++

CPPFLAGS := -I.

COMMON_FLAGS := -Og -ggdb3 -ftrapv -pedantic \
	-Wall \
	-Wextra \
	-Wdouble-promotion \
	-Wformat=2 \
	-Wformat-overflow=2 \
	-Wmisleading-indentation \
	-Wmultistatement-macros \
	-Wstrict-aliasing \
	-Wsuggest-attribute=pure \
	-Wsuggest-attribute=const \
	-Wsuggest-attribute=malloc \
	-Wsuggest-attribute=format \
	-Wvla-parameter \
	-Warray-bounds \
	-Wduplicated-branches \
	-Wduplicated-cond \
	-Wtrampolines \
	-Wfloat-equal \
	-Wshadow=global \
	-Wpedantic \
	-Wcast-qual \
	-Wcast-align \
	-Wconversion \
	-Warith-conversion \
	-Wlogical-op \
	-Warray-parameter \
	-Wno-float-equal \
	-Wno-c99-extension

ASAN_FLAGS := \
	$(COMMON_FLAGS) \
	-fsanitize=address \
	-fsanitize=pointer-compare \
	-fsanitize=pointer-subtract \
	-fsanitize=leak

UBSAN_FLAGS := \
	$(COMMON_FLAGS) \
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
	-fsanitize=pointer-overflow

ANALYZER_FLAGS := \
	$(COMMON_FLAGS) \
	-fanalyzer \
	-Wanalyzer-too-complex

# gcc only
#	-fsanitize=bounds-strict \

CFLAGS := -O2

CXXFLAGS := -O2
CXXFLAGS += -std=c++11

LDFLAGS :=

all: test_predicate address_sanitizer undefined_sanitizer analyzer

address_sanitizer: test_predicate_asan
	$(addprefix ./,$<)

undefined_sanitizer: test_predicate_ubsan
	$(addprefix ./,$<)

analyzer: test_predicate_analyzer
	$(addprefix ./,$<)

test_predicate_asan: LDFLAGS += $(ASAN_FLAGS)
test_predicate_asan: test_predicate_asan.o
	$(CXXLD) $(LDFLAGS) -o $@ $^

test_predicate_ubsan: LDFLAGS += $(UBSAN_FLAGS)
test_predicate_ubsan: test_predicate_ubsan.o
	$(CXXLD) $(LDFLAGS) -o $@ $^

test_predicate_analyzer: LDFLAGS += $(ANALYZER_FLAGS)
test_predicate_analyzer: test_predicate_analyzer.o
	$(CXXLD) $(LDFLAGS) -o $@ $^

%_asan.o: CXXFLAGS += $(ASAN_FLAGS)
%_asan.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

%_ubsan.o: CXXFLAGS += $(UBSAN_FLAGS)
%_ubsan.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

%_analyzer.o: CXXFLAGS += $(ANALYZER_FLAGS)
%_analyzer.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

clean:
	rm test_predicate test_predicate_asan test_predicate_ubsan test_predicate_analyzer *.o
