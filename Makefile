CPP := gcc
CC := gcc
CXX := g++
CXXLD := g++

RM := rm -f
RMDIR := rm -rf
MKDIR := mkdir -p

BUILD_DIR := build

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
	-fvar-tracking-assignments \
	-ftrapv \
	-pedantic \
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
	-Wno-float-equal

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
	-fsanitize=pointer-overflow

ANALYZER_FLAGS := \
	-fanalyzer \
	-Wanalyzer-too-complex

# gcc only
#	-fsanitize=bounds-strict \

CFLAGS := \
	-std=c11 \
	$(COMMON_FLAGS)

CXXFLAGS := \
	-std=c++11 \
	$(COMMON_FLAGS)

LDFLAGS := -rdynamic

LDLIBS := -lunwind

all: $(BUILD_DIR)/test_predicate

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

$(BUILD_DIR)/%_asan.o: %.cpp | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

$(BUILD_DIR)/%_ubsan.o: %.cpp | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

$(BUILD_DIR)/%_analyzer.o: %.cpp | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: %.cpp | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

clean:
	$(RMDIR) $(BUILD_DIR)

$(BUILD_DIR):
	$(MKDIR) $@

.PHONY: clean

-include $(BUILD_DIR)/*.d
