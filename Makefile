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
TARGET = bin/main

CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -pthread -O2

.PHONY: all clean

all: $(TARGET)

clean:
	rm -f $(OBJECTS) $(TARGET); \
	> logs/alpha_log.txt; \
	> logs/calculation.log

# Create directories
obj:
	mkdir -p $@

# Pattern rule for object files
obj/%.o: src/%.cpp | obj
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Dependencies (add these if you want automatic header dependency tracking)
obj/alpha_calc/calculation_logger.o: src/alpha_calc/calculation_logger.cpp src/alpha_calc/calculation_logger.hpp src/alpha_calc/ring_buffer_queue.hpp
obj/alpha_calc/constants.o: src/alpha_calc/constants.cpp src/alpha_calc/constants.hpp
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
obj/main.o: src/main.cpp src/number_theory/prime_generator.hpp src/alpha_calc/important_funcs.hpp src/www_nim_calc/ww.hpp src/www_nim_calc/www.hpp src/www_nim_calc/www_nim.hpp