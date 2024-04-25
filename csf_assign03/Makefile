
CONSERVATIVE_FLAGS = -std=c++11 -Wall -Wextra -pedantic
DEBUGGING_FLAGS = -g -O0
CFLAGS = $(CONSERVATIVE_FLAGS) $(DEBUGGING_FLAGS)

csim : csim.o cache.o 
	g++ csim.o cache.o -o csim

cache.o : cache.cpp cache.h
	g++ $(CFLAGS) -c cache.cpp

csim.o : csim.cpp
	g++ $(CFLAGS) -c csim.cpp

.PHONY: solution.zip
solution.zip:
	rm -f $@
	zip -9r $@ *.h *.cpp *.S Makefile README.txt

# Clean the build directory
.PHONY: clean
clean:
	rm -f *.o csim solution.zip
	
