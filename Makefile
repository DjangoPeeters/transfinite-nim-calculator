# Mostly written by Claude AI
SOURCES = src/alpha_calc/calculation_logger.cpp \
		  src/alpha_calc/constants.cpp \
		  src/alpha_calc/impartial_term_algebra.cpp \
		  src/alpha_calc/important_funcs.cpp \
		  src/alpha_calc/ring_buffer_queue.cpp \
		  src/number_theory/nt_funcs.cpp \
		  src/number_theory/prime_generator.cpp \
		  src/www_nim_calc/fin_nim.cpp \
		  src/www_nim_calc/kappa_component.cpp \
		  src/www_nim_calc/ww.cpp \
		  src/www_nim_calc/www_nim.cpp \
		  src/www_nim_calc/www.cpp \
		  src/misc.cpp \
		  src/main.cpp
TEST_SOURCES = test/test.cpp
OBJECTS = $(SOURCES:src/%.cpp=obj/%.o)
DEBUG_OBJECTS = $(SOURCES:src/%.cpp=obj/debug/%.o)
PROF_OBJECTS = $(SOURCES:src/%.cpp=obj/prof/%.o)
PERF_OBJECTS = $(SOURCES:src/%.cpp=obj/perf/%.o)
TEST_OBJECTS = $(TEST_SOURCES:test/%.cpp=obj/test/%.o)
TARGET = bin/main

CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -pthread
# TEST_MODE controls important_funcs.cpp's TEST_MODE constant (see there for what it does —
# short version: 1 starts every cache empty for benchmarking, 0 uses the real accumulated
# knowledge in logs/*_records.txt for real work). Override with `make TEST_MODE=0`, which only
# affects the release build ($(TARGET)) — debug/profile/perf/test all force TEST_MODE=1
# unconditionally, since caches serving cached values instead of computing them defeats the
# purpose of testing, debugging, and profiling the computation itself.
# CAUTION: object files are keyed by path, not by which flags built them, so switching this
# value does NOT force a recompile on its own — `make clean` first, or you'll silently link
# object files built with the OLD value.
TEST_MODE ?= 1
# Separate targets for release and profiling
RELEASE_FLAGS = -O3 -DNDEBUG -mpopcnt -DTEST_MODE_ENABLED=$(TEST_MODE)
DEBUG_FLAGS = -fsanitize=address -g -O0 -DDEBUG -DTEST_MODE_ENABLED=1
PROFILE_FLAGS = -pg -O3 -DNDEBUG -mpopcnt -DTEST_MODE_ENABLED=1
TEST_FLAGS = -O3 -DNDEBUG -mpopcnt -DTEST_MODE_ENABLED=1
# Same optimization as the release build (this is what we want to characterize), plus debug
# symbols so `perf report`/`perf annotate` can resolve function and line info, plus explicit
# frame pointers so `perf record --call-graph fp` can unwind call stacks without the extra
# per-sample overhead of DWARF-based unwinding. No -pg here: gprof's own call-counting
# instrumentation adds a per-call cost that disproportionately distorts very-high-call-count
# functions (we saw this firsthand with q_power_times_term_calc) — perf's sampling approach
# doesn't have that problem, which is part of why it's worth using on top of gprof.
PERF_FLAGS = -O3 -DNDEBUG -mpopcnt -g -fno-omit-frame-pointer -DTEST_MODE_ENABLED=1

#TODO use include directory for proper dependency handling

# Default target (release build)
all: $(TARGET)

# Create necessary directories
bin:
	mkdir -p bin

obj:
	mkdir -p obj

obj/debug:
	mkdir -p obj/debug

obj/prof:
	mkdir -p obj/prof

obj/perf:
	mkdir -p obj/perf

obj/test:
	mkdir -p obj/test

# Release build
$(TARGET): $(OBJECTS) | bin
	$(CXX) $(CXXFLAGS) $(RELEASE_FLAGS) $(OBJECTS) -o $(TARGET)

# Release object files
obj/%.o: src/%.cpp | obj
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(RELEASE_FLAGS) -c $< -o $@

# Debug build
$(TARGET)_debug: $(DEBUG_OBJECTS) | bin
	$(CXX) $(CXXFLAGS) $(DEBUG_FLAGS) $(DEBUG_OBJECTS) -o $(TARGET)_debug

# Debug object files
obj/debug/%.o: src/%.cpp | obj/debug
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(DEBUG_FLAGS) -c $< -o $@

# Profile build
$(TARGET)_prof: $(PROF_OBJECTS) | bin
	$(CXX) $(CXXFLAGS) $(PROFILE_FLAGS) $(PROF_OBJECTS) -o $(TARGET)_prof

# Profile object files
obj/prof/%.o: src/%.cpp | obj/prof
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(PROFILE_FLAGS) -c $< -o $@

# perf build
$(TARGET)_perf: $(PERF_OBJECTS) | bin
	$(CXX) $(CXXFLAGS) $(PERF_FLAGS) $(PERF_OBJECTS) -o $(TARGET)_perf

# perf object files
obj/perf/%.o: src/%.cpp | obj/perf
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(PERF_FLAGS) -c $< -o $@

# Test build
$(TARGET)_test: $(OBJECTS) $(TEST_OBJECTS) | bin
	$(CXX) $(CXXFLAGS) $(TEST_FLAGS) $(TEST_OBJECTS) $(subst obj/main.o,,$(OBJECTS)) -o $(TARGET)_test

# Test object files
obj/test/%.o: test/%.cpp | obj/test
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(TEST_FLAGS) -c $< -o $@

# Debug build shortcut
debug: $(TARGET)_debug
	@echo "Debug build complete: $(TARGET)_debug"
	@echo "Run with: ./$(TARGET)_debug"
	@echo "Debug with gdb: gdb ./$(TARGET)_debug"

# Debug and run immediately  
run-debug: $(TARGET)_debug
	@echo "Running debug version..."
	./$(TARGET)_debug

# Debug with gdb
gdb-debug: $(TARGET)_debug
	@echo "Starting GDB..."
	gdb ./$(TARGET)_debug

# Debug with Valgrind (memory checking)
valgrind-debug: $(TARGET)_debug
	@echo "Running with Valgrind (memory check)..."
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(TARGET)_debug

valgrind-release: $(TARGET)
	@echo "Running release build with Valgrind..."
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(TARGET)

# Run profiling and generate report
profile: $(TARGET)_prof
	@echo "Running profiling..."
	./$(TARGET)_prof
	@echo "Generating profile report..."
	gprof $(TARGET)_prof gmon.out > profile_report.txt
	@echo "Profile report saved to profile_report.txt"
	@echo ""
	@echo "=== TOP FUNCTIONS ==="
	@grep -A 10 "Each sample counts" profile_report.txt | head -15

# Quick profile with call graph
profile-detailed: $(TARGET)_prof
	@echo "Running detailed profiling..."
	./$(TARGET)_prof
	@echo "Generating detailed profile report..."
	gprof $(TARGET)_prof gmon.out > profile_detailed.txt
	@echo "=== FLAT PROFILE ==="
	@grep -A 15 "Each sample counts" profile_detailed.txt | head -20
	@echo ""
	@echo "=== CALL GRAPH (top functions) ==="
	@grep -A 20 "Call graph" profile_detailed.txt | head -25

# Quick profile - just show essentials
quick-profile: $(TARGET)_prof
	@echo "Running quick profile..."
	./$(TARGET)_prof
	@echo "=== TOP TIME-CONSUMING FUNCTIONS ==="
	@gprof $(TARGET)_prof gmon.out | head -25

# perf build shortcut. Unlike profile/profile-detailed above, this doesn't run perf itself —
# which prime to run, for how long, and which counters to watch are all things you'll want to
# choose per investigation, not bake into the Makefile.
perf-build: $(TARGET)_perf
	@echo "perf build complete: $(TARGET)_perf"
	@echo ""
	@echo "First, check whether sampling is allowed on this node:"
	@echo "  cat /proc/sys/kernel/perf_event_paranoid"
	@echo "  (0-1 = fine; 2 restricts kernel-level sampling but perf stat on your own process"
	@echo "   still works; if it errors out entirely, ask the sysadmins or fall back to perf stat)"
	@echo ""
	@echo "Aggregate counters for one run (fast, low overhead, good first check):"
	@echo "  perf stat -e cycles,instructions,cache-references,cache-misses,LLC-loads,LLC-load-misses,dTLB-load-misses,branch-misses -- ./$(TARGET)_perf alpha logs <p>"
	@echo ""
	@echo "Full sampled profile with call graph (bigger perf.data, needed for perf report):"
	@echo "  perf record -g --call-graph fp -o perf.data -- ./$(TARGET)_perf alpha logs <p>"
	@echo "  perf report -i perf.data"
	@echo "  (if fp-based call graphs look truncated/unreliable, retry with --call-graph dwarf —"
	@echo "   heavier, but a more reliable fallback if frame pointers get lost somewhere)"
	@echo ""
	@echo "Line-level attribution within a hot function once you know which one to look at:"
	@echo "  perf annotate -i perf.data square_with_table"

# Test build shortcut
test: $(TARGET)_test
	@echo "Test build complete: $(TARGET)_test"
	@echo "Run with: ./$(TARGET)_test"

# Show file structure (helpful for debugging)
show-files:
	@echo "Source files found:"
	@echo "$(SOURCES)" | tr ' ' '\n'
	@echo ""
	@echo "Object files (debug):"
	@echo "$(DEBUG_OBJECTS)" | tr ' ' '\n'
	@echo ""
	@echo "Object files (release):"
	@echo "$(OBJECTS)" | tr ' ' '\n'
	@echo ""
	@echo "Object files (profile):"
	@echo "$(PROF_OBJECTS)" | tr ' ' '\n'
	@echo ""
	@echo "Object files (perf):"
	@echo "$(PERF_OBJECTS)" | tr ' ' '\n'
	@echo ""
	@echo "Test source files found:"
	@echo "$(SOURCES)" | tr ' ' '\n'
	@echo ""
	@echo "Object files (test):"
	@echo "$(TEST_OBJECTS)" | tr ' ' '\n'

# Clean up all generated files
clean:
	rm -rf obj $(TARGET) $(TARGET)_debug $(TARGET)_prof $(TARGET)_perf gmon.out profile_report.txt profile_detailed.txt perf.data perf.data.old; \
	> logs/calculation.log

# Clean only debug files
clean-debug:
	rm -rf obj/debug $(TARGET)_debug

# Clean only profiling files
clean-profile:
	rm -rf obj/prof $(TARGET)_prof gmon.out profile_report.txt profile_detailed.txt

# Clean only perf files
clean-perf:
	rm -rf obj/perf $(TARGET)_perf perf.data perf.data.old

# Clean only test files
clean-test:
	rm -rf obj/test $(TARGET)test

# Clean and rebuild everything
rebuild: clean all

# Clean and rebuild debug version
rebuild-debug: clean-debug debug

# Clean and rebuild profile version
rebuild-profile: clean-profile profile

# Clean and rebuild perf version
rebuild-perf: clean-perf perf-build

# Clean and rebuild debug version
rebuild-test: clean-test test

.PHONY: all debug run-debug gdb-debug valgrind-debug profile profile-detailed quick-profile perf-build test show-files clean clean-debug clean-profile clean-perf clean-test rebuild rebuild-debug rebuild-profile rebuild-perf rebuild-test

# Dependencies
obj/alpha_calc/calculation_logger.o: src/alpha_calc/calculation_logger.cpp src/alpha_calc/calculation_logger.hpp src/alpha_calc/ring_buffer_queue.hpp
obj/alpha_calc/constants.o: src/alpha_calc/constants.cpp src/alpha_calc/constants.hpp src/alpha_calc/calculation_logger.hpp
obj/alpha_calc/impartial_term_algebra.o: src/alpha_calc/impartial_term_algebra.cpp src/alpha_calc/impartial_term_algebra.hpp src/number_theory/nt_funcs.hpp src/alpha_calc/important_funcs.hpp
obj/alpha_calc/important_funcs.o: src/alpha_calc/important_funcs.cpp src/alpha_calc/important_funcs.hpp src/number_theory/nt_funcs.hpp src/alpha_calc/impartial_term_algebra.hpp src/alpha_calc/constants.hpp src/alpha_calc/ring_buffer_queue.hpp src/alpha_calc/calculation_logger.hpp src/misc.hpp
obj/alpha_calc/ring_buffer_queue.o: src/alpha_calc/ring_buffer_queue.cpp src/alpha_calc/ring_buffer_queue.hpp
obj/number_theory/nt_funcs.o: src/number_theory/nt_funcs.cpp src/number_theory/nt_funcs.hpp src/number_theory/prime_generator.hpp
obj/number_theory/prime_generator.o: src/number_theory/prime_generator.cpp src/number_theory/prime_generator.hpp
obj/www_nim_calc/fin_nim.o: src/www_nim_calc/fin_nim.cpp src/www_nim_calc/fin_nim.hpp
obj/www_nim_calc/kappa_component.o: src/www_nim_calc/kappa_component.cpp src/www_nim_calc/kappa_component.hpp src/number_theory/prime_generator.hpp
obj/www_nim_calc/ww.o: src/www_nim_calc/ww.cpp src/www_nim_calc/ww.hpp
obj/www_nim_calc/www_nim.o: src/www_nim_calc/www_nim.cpp src/www_nim_calc/www_nim.hpp src/number_theory/prime_generator.hpp src/number_theory/nt_funcs.hpp src/alpha_calc/important_funcs.hpp src/www_nim_calc/fin_nim.hpp src/www_nim_calc/ww.hpp src/www_nim_calc/www.hpp src/www_nim_calc/kappa_component.hpp
obj/www_nim_calc/www.o: src/www_nim_calc/www.cpp src/www_nim_calc/www.hpp src/www_nim_calc/ww.hpp
obj/misc.o: src/misc.cpp src/misc.hpp
obj/main.o: src/main.cpp src/alpha_calc/calculation_logger.hpp src/alpha_calc/important_funcs.hpp src/number_theory/prime_generator.hpp src/www_nim_calc/ww.hpp src/www_nim_calc/www.hpp src/www_nim_calc/www_nim.hpp src/misc.hpp

obj/debug/alpha_calc/calculation_logger.o: src/alpha_calc/calculation_logger.cpp src/alpha_calc/calculation_logger.hpp src/alpha_calc/ring_buffer_queue.hpp
obj/debug/alpha_calc/constants.o: src/alpha_calc/constants.cpp src/alpha_calc/constants.hpp src/alpha_calc/calculation_logger.hpp
obj/debug/alpha_calc/impartial_term_algebra.o: src/alpha_calc/impartial_term_algebra.cpp src/alpha_calc/impartial_term_algebra.hpp src/number_theory/nt_funcs.hpp src/alpha_calc/important_funcs.hpp
obj/debug/alpha_calc/important_funcs.o: src/alpha_calc/important_funcs.cpp src/alpha_calc/important_funcs.hpp src/number_theory/nt_funcs.hpp src/alpha_calc/impartial_term_algebra.hpp src/alpha_calc/constants.hpp src/alpha_calc/ring_buffer_queue.hpp src/alpha_calc/calculation_logger.hpp src/misc.hpp
obj/debug/alpha_calc/ring_buffer_queue.o: src/alpha_calc/ring_buffer_queue.cpp src/alpha_calc/ring_buffer_queue.hpp
obj/debug/number_theory/nt_funcs.o: src/number_theory/nt_funcs.cpp src/number_theory/nt_funcs.hpp src/number_theory/prime_generator.hpp
obj/debug/number_theory/prime_generator.o: src/number_theory/prime_generator.cpp src/number_theory/prime_generator.hpp
obj/debug/www_nim_calc/fin_nim.o: src/www_nim_calc/fin_nim.cpp src/www_nim_calc/fin_nim.hpp
obj/debug/www_nim_calc/kappa_component.o: src/www_nim_calc/kappa_component.cpp src/www_nim_calc/kappa_component.hpp src/number_theory/prime_generator.hpp
obj/debug/www_nim_calc/ww.o: src/www_nim_calc/ww.cpp src/www_nim_calc/ww.hpp
obj/debug/www_nim_calc/www_nim.o: src/www_nim_calc/www_nim.cpp src/www_nim_calc/www_nim.hpp src/number_theory/prime_generator.hpp src/number_theory/nt_funcs.hpp src/alpha_calc/important_funcs.hpp src/www_nim_calc/fin_nim.hpp src/www_nim_calc/ww.hpp src/www_nim_calc/www.hpp src/www_nim_calc/kappa_component.hpp
obj/debug/www_nim_calc/www.o: src/www_nim_calc/www.cpp src/www_nim_calc/www.hpp src/www_nim_calc/ww.hpp
obj/debug/misc.o: src/misc.cpp src/misc.hpp
obj/debug/main.o: src/main.cpp src/alpha_calc/calculation_logger.hpp src/alpha_calc/important_funcs.hpp src/number_theory/prime_generator.hpp src/www_nim_calc/ww.hpp src/www_nim_calc/www.hpp src/www_nim_calc/www_nim.hpp src/misc.hpp

obj/prof/alpha_calc/calculation_logger.o: src/alpha_calc/calculation_logger.cpp src/alpha_calc/calculation_logger.hpp src/alpha_calc/ring_buffer_queue.hpp
obj/prof/alpha_calc/constants.o: src/alpha_calc/constants.cpp src/alpha_calc/constants.hpp src/alpha_calc/calculation_logger.hpp
obj/prof/alpha_calc/impartial_term_algebra.o: src/alpha_calc/impartial_term_algebra.cpp src/alpha_calc/impartial_term_algebra.hpp src/number_theory/nt_funcs.hpp src/alpha_calc/important_funcs.hpp
obj/prof/alpha_calc/important_funcs.o: src/alpha_calc/important_funcs.cpp src/alpha_calc/important_funcs.hpp src/number_theory/nt_funcs.hpp src/alpha_calc/impartial_term_algebra.hpp src/alpha_calc/constants.hpp src/alpha_calc/ring_buffer_queue.hpp src/alpha_calc/calculation_logger.hpp src/misc.hpp
obj/prof/alpha_calc/ring_buffer_queue.o: src/alpha_calc/ring_buffer_queue.cpp src/alpha_calc/ring_buffer_queue.hpp
obj/prof/number_theory/nt_funcs.o: src/number_theory/nt_funcs.cpp src/number_theory/nt_funcs.hpp src/number_theory/prime_generator.hpp
obj/prof/number_theory/prime_generator.o: src/number_theory/prime_generator.cpp src/number_theory/prime_generator.hpp
obj/prof/www_nim_calc/fin_nim.o: src/www_nim_calc/fin_nim.cpp src/www_nim_calc/fin_nim.hpp
obj/prof/www_nim_calc/kappa_component.o: src/www_nim_calc/kappa_component.cpp src/www_nim_calc/kappa_component.hpp src/number_theory/prime_generator.hpp
obj/prof/www_nim_calc/ww.o: src/www_nim_calc/ww.cpp src/www_nim_calc/ww.hpp
obj/prof/www_nim_calc/www_nim.o: src/www_nim_calc/www_nim.cpp src/www_nim_calc/www_nim.hpp src/number_theory/prime_generator.hpp src/number_theory/nt_funcs.hpp src/alpha_calc/important_funcs.hpp src/www_nim_calc/fin_nim.hpp src/www_nim_calc/ww.hpp src/www_nim_calc/www.hpp src/www_nim_calc/kappa_component.hpp
obj/prof/www_nim_calc/www.o: src/www_nim_calc/www.cpp src/www_nim_calc/www.hpp src/www_nim_calc/ww.hpp
obj/prof/misc.o: src/misc.cpp src/misc.hpp
obj/prof/main.o: src/main.cpp src/alpha_calc/calculation_logger.hpp src/alpha_calc/important_funcs.hpp src/number_theory/prime_generator.hpp src/www_nim_calc/ww.hpp src/www_nim_calc/www.hpp src/www_nim_calc/www_nim.hpp src/misc.hpp

obj/perf/alpha_calc/calculation_logger.o: src/alpha_calc/calculation_logger.cpp src/alpha_calc/calculation_logger.hpp src/alpha_calc/ring_buffer_queue.hpp
obj/perf/alpha_calc/constants.o: src/alpha_calc/constants.cpp src/alpha_calc/constants.hpp src/alpha_calc/calculation_logger.hpp
obj/perf/alpha_calc/impartial_term_algebra.o: src/alpha_calc/impartial_term_algebra.cpp src/alpha_calc/impartial_term_algebra.hpp src/number_theory/nt_funcs.hpp src/alpha_calc/important_funcs.hpp
obj/perf/alpha_calc/important_funcs.o: src/alpha_calc/important_funcs.cpp src/alpha_calc/important_funcs.hpp src/number_theory/nt_funcs.hpp src/alpha_calc/impartial_term_algebra.hpp src/alpha_calc/constants.hpp src/alpha_calc/ring_buffer_queue.hpp src/alpha_calc/calculation_logger.hpp src/misc.hpp
obj/perf/alpha_calc/ring_buffer_queue.o: src/alpha_calc/ring_buffer_queue.cpp src/alpha_calc/ring_buffer_queue.hpp
obj/perf/number_theory/nt_funcs.o: src/number_theory/nt_funcs.cpp src/number_theory/nt_funcs.hpp src/number_theory/prime_generator.hpp
obj/perf/number_theory/prime_generator.o: src/number_theory/prime_generator.cpp src/number_theory/prime_generator.hpp
obj/perf/www_nim_calc/fin_nim.o: src/www_nim_calc/fin_nim.cpp src/www_nim_calc/fin_nim.hpp
obj/perf/www_nim_calc/kappa_component.o: src/www_nim_calc/kappa_component.cpp src/www_nim_calc/kappa_component.hpp src/number_theory/prime_generator.hpp
obj/perf/www_nim_calc/ww.o: src/www_nim_calc/ww.cpp src/www_nim_calc/ww.hpp
obj/perf/www_nim_calc/www_nim.o: src/www_nim_calc/www_nim.cpp src/www_nim_calc/www_nim.hpp src/number_theory/prime_generator.hpp src/number_theory/nt_funcs.hpp src/alpha_calc/important_funcs.hpp src/www_nim_calc/fin_nim.hpp src/www_nim_calc/ww.hpp src/www_nim_calc/www.hpp src/www_nim_calc/kappa_component.hpp
obj/perf/www_nim_calc/www.o: src/www_nim_calc/www.cpp src/www_nim_calc/www.hpp src/www_nim_calc/ww.hpp
obj/perf/misc.o: src/misc.cpp src/misc.hpp
obj/perf/main.o: src/main.cpp src/alpha_calc/calculation_logger.hpp src/alpha_calc/important_funcs.hpp src/number_theory/prime_generator.hpp src/www_nim_calc/ww.hpp src/www_nim_calc/www.hpp src/www_nim_calc/www_nim.hpp src/misc.hpp

obj/test/test.o: test/test.cpp \
				 test/acutest.h \
				 src/alpha_calc/calculation_logger.hpp \
				 src/alpha_calc/constants.hpp \
				 src/alpha_calc/impartial_term_algebra.hpp \
				 src/alpha_calc/important_funcs.hpp \
				 src/alpha_calc/ring_buffer_queue.hpp \
				 src/number_theory/nt_funcs.hpp \
				 src/number_theory/prime_generator.hpp \
				 src/www_nim_calc/fin_nim.hpp \
				 src/www_nim_calc/kappa_component.hpp \
				 src/www_nim_calc/ww.hpp \
				 src/www_nim_calc/www_nim.hpp \
				 src/www_nim_calc/www.hpp \
				 src/misc.hpp
