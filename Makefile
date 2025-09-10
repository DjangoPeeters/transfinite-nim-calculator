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
		  src/main.cpp
OBJECTS = $(SOURCES:src/%.cpp=obj/%.o)
PROF_OBJECTS = $(SOURCES:src/%.cpp=obj/prof/%.o)
TARGET = bin/main

CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -pthread
# Separate targets for release and profiling
RELEASE_FLAGS = -O2
PROFILE_FLAGS = -pg -O2

all: $(TARGET)

# Create necessary directories
obj:
	mkdir -p obj

obj/prof:
	mkdir -p obj/prof

# Release build
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(RELEASE_FLAGS) $(OBJECTS) -o $(TARGET)

# Release object files
obj/%.o: src/%.cpp | obj
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(RELEASE_FLAGS) -c $< -o $@

# Profile build
$(TARGET)_prof: $(PROF_OBJECTS)
	$(CXX) $(CXXFLAGS) $(PROFILE_FLAGS) $(PROF_OBJECTS) -o $(TARGET)_prof

# Profile object files
obj/prof/%.o: src/%.cpp | obj/prof
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(PROFILE_FLAGS) -c $< -o $@

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

# Show file structure (helpful for debugging)
show-files:
	@echo "Source files found:"
	@echo "$(SOURCES)" | tr ' ' '\n'
	@echo ""
	@echo "Object files (release):"
	@echo "$(OBJECTS)" | tr ' ' '\n'
	@echo ""
	@echo "Object files (profile):"
	@echo "$(PROF_OBJECTS)" | tr ' ' '\n'

# Clean up all generated files
clean:
	rm -rf obj $(TARGET) $(TARGET)_prof gmon.out profile_report.txt profile_detailed.txt; \
	> logs/alpha_log.txt; \
	> logs/calculation.log

# Clean only profiling files
clean-profile:
	rm -rf obj/prof $(TARGET)_prof gmon.out profile_report.txt profile_detailed.txt

# Clean and rebuild everything
rebuild: clean all

# Clean and rebuild profile version
rebuild-profile: clean-profile profile

.PHONY: all profile profile-detailed quick-profile show-files clean clean-profile rebuild rebuild-profile

# Dependencies
obj/alpha_calc/calculation_logger.o: src/alpha_calc/calculation_logger.cpp src/alpha_calc/calculation_logger.hpp src/alpha_calc/ring_buffer_queue.hpp
obj/alpha_calc/constants.o: src/alpha_calc/constants.cpp src/alpha_calc/constants.hpp src/alpha_calc/calculation_logger.hpp
obj/alpha_calc/impartial_term_algebra.o: src/alpha_calc/impartial_term_algebra.cpp src/alpha_calc/impartial_term_algebra.hpp src/number_theory/nt_funcs.hpp src/alpha_calc/important_funcs.hpp
obj/alpha_calc/important_funcs.o: src/alpha_calc/important_funcs.cpp src/alpha_calc/important_funcs.hpp src/number_theory/nt_funcs.hpp src/alpha_calc/impartial_term_algebra.hpp src/alpha_calc/constants.hpp src/alpha_calc/ring_buffer_queue.hpp src/alpha_calc/calculation_logger.hpp
obj/alpha_calc/ring_buffer_queue.o: src/alpha_calc/ring_buffer_queue.cpp src/alpha_calc/ring_buffer_queue.hpp
obj/number_theory/nt_funcs.o: src/number_theory/nt_funcs.cpp src/number_theory/nt_funcs.hpp src/number_theory/prime_generator.hpp
obj/number_theory/prime_generator.o: src/number_theory/prime_generator.cpp src/number_theory/prime_generator.hpp
obj/www_nim_calc/fin_nim.o: src/www_nim_calc/fin_nim.cpp src/www_nim_calc/fin_nim.hpp
obj/www_nim_calc/kappa_component.o: src/www_nim_calc/kappa_component.cpp src/www_nim_calc/kappa_component.hpp src/number_theory/prime_generator.hpp
obj/www_nim_calc/ww.o: src/www_nim_calc/ww.cpp src/www_nim_calc/ww.hpp
obj/www_nim_calc/www_nim.o: src/www_nim_calc/www_nim.cpp src/www_nim_calc/www_nim.hpp src/number_theory/prime_generator.hpp src/number_theory/nt_funcs.hpp src/alpha_calc/important_funcs.hpp src/www_nim_calc/fin_nim.hpp src/www_nim_calc/ww.hpp src/www_nim_calc/www.hpp src/www_nim_calc/kappa_component.hpp
obj/www_nim_calc/www.o: src/www_nim_calc/www.cpp src/www_nim_calc/www.hpp src/www_nim_calc/ww.hpp
obj/main.o: src/main.cpp src/alpha_calc/calculation_logger.hpp src/alpha_calc/important_funcs.hpp src/number_theory/prime_generator.hpp src/www_nim_calc/ww.hpp src/www_nim_calc/www.hpp src/www_nim_calc/www_nim.hpp