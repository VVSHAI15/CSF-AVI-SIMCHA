# Makefile for CSF Assignment 3

CONSERVATIVE_FLAGS = -std=c++11 -Wall -Wextra -pedantic
DEBUGGING_FLAGS = -g -O0
CFLAGS = $(CONSERVATIVE_FLAGS) $(DEBUGGING_FLAGS)

# Target for csim executable
csim: csim.o
	g++ -o csim csim.o $(CFLAGS)

# Compile csim.cpp to csim.o
csim.o: csim.cpp
	g++ -c csim.cpp $(CFLAGS)

.PHONY: solution.zip
solution.zip:
	rm -f $@
	zip -9r $@ *.h *.cpp *.S Makefile README.txt

# Clean the build directory
.PHONY: clean
clean:
	rm -f *.o csim solution.zip