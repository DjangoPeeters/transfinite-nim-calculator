# Mostly written by Claude AI
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -pthread -O2
TARGET = main
SOURCES = main.cpp calculation_logger.cpp constants.cpp fin_nim.cpp impartial_term_algebra.cpp important_funcs.cpp kappa_component.cpp nt_funcs.cpp periodic_bools.cpp prime_generator.cpp ring_buffer_queue.cpp ww.cpp www_nim.cpp www.cpp
OBJECTS = $(SOURCES:.cpp=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET) calculation.log

# Dependencies (add these if you want automatic header dependency tracking)
main.o: main.cpp prime_generator.hpp important_funcs.hpp ww.hpp www.hpp www_nim.hpp
prime_generator.o: prime_generator.cpp prime_generator.hpp
important_funcs.o: important_funcs.cpp important_funcs.hpp nt_funcs.hpp impartial_term_algebra.hpp constants.hpp ring_buffer_queue.hpp calculation_logger.hpp
ww.o: ww.cpp ww.hpp
www.o: www.cpp www.hpp ww.hpp
www_nim.o: www_nim.cpp www_nim.hpp prime_generator.hpp nt_funcs.hpp important_funcs.hpp fin_nim.hpp ww.hpp www.hpp kappa_component.hpp
nt_funcs.o: nt_funcs.cpp nt_funcs.hpp prime_generator.hpp
impartial_term_algebra.o: impartial_term_algebra.cpp impartial_term_algebra.hpp nt_funcs.hpp important_funcs.hpp periodic_bools.hpp
constants.o: constants.cpp constants.hpp
ring_buffer_queue.o: ring_buffer_queue.cpp ring_buffer_queue.hpp
calculation_logger.o: calculation_logger.cpp calculation_logger.hpp ring_buffer_queue.hpp
fin_nim.o: fin_nim.cpp fin_nim.hpp
kappa_component.o: kappa_component.cpp kappa_component.hpp prime_generator.hpp
periodic_bools.o: periodic_bools.cpp periodic_bools.hpp