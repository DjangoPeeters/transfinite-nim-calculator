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
TEST_OBJECTS = $(TEST_SOURCES:test/%.cpp=obj/test/%.o)
TARGET = bin/main

CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -pthread
# Separate targets for release and profiling
RELEASE_FLAGS = -O3 -DNDEBUG
DEBUG_FLAGS = -fsanitize=address -g -O0 -DDEBUG
PROFILE_FLAGS = -pg -O2
TEST_FLAGS = -O3 -DNDEBUG

#TODO use include directory for proper dependency handling

# Default target (release build)
all: $(TARGET)

# Create necessary directories
obj:
	mkdir -p obj

obj/debug:
	mkdir -p obj/debug

obj/prof:
	mkdir -p obj/prof

obj/test:
	mkdir -p obj/test

# Release build
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(RELEASE_FLAGS) $(OBJECTS) -o $(TARGET)

# Release object files
obj/%.o: src/%.cpp | obj
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(RELEASE_FLAGS) -c $< -o $@

# Debug build
$(TARGET)_debug: $(DEBUG_OBJECTS)
	$(CXX) $(CXXFLAGS) $(DEBUG_FLAGS) $(DEBUG_OBJECTS) -o $(TARGET)_debug

# Debug object files
obj/debug/%.o: src/%.cpp | obj/debug
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(DEBUG_FLAGS) -c $< -o $@

# Profile build
$(TARGET)_prof: $(PROF_OBJECTS)
	$(CXX) $(CXXFLAGS) $(PROFILE_FLAGS) $(PROF_OBJECTS) -o $(TARGET)_prof

# Profile object files
obj/prof/%.o: src/%.cpp | obj/prof
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(PROFILE_FLAGS) -c $< -o $@

# Test build
$(TARGET)_test: $(TEST_OBJECTS) | $(OBJECTS)
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
	@echo "Test source files found:"
	@echo "$(SOURCES)" | tr ' ' '\n'
	@echo ""
	@echo "Object files (test):"
	@echo "$(TEST_OBJECTS)" | tr ' ' '\n'

# Clean up all generated files
clean:
	rm -rf obj $(TARGET) $(TARGET)_debug $(TARGET)_prof gmon.out profile_report.txt profile_detailed.txt; \
	> logs/calculation.log

# Clean only debug files
clean-debug:
	rm -rf obj/debug $(TARGET)_debug

# Clean only profiling files
clean-profile:
	rm -rf obj/prof $(TARGET)_prof gmon.out profile_report.txt profile_detailed.txt

# Clean only test files
clean-test:
	rm -rf obj/test $(TARGET)test

# Clean and rebuild everything
rebuild: clean all

# Clean and rebuild debug version
rebuild-debug: clean-debug debug

# Clean and rebuild profile version
rebuild-profile: clean-profile profile

# Clean and rebuild debug version
rebuild-test: clean-test test

.PHONY: all debug run-debug gdb-debug valgrind-debug profile profile-detailed quick-profile test show-files clean clean-debug clean-profile clean-test rebuild rebuild-debug rebuild-profile rebuild-test

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

obj/test/test.o: test/test.cpp test/acutest.h
