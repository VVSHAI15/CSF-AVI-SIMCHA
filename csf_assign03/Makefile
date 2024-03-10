# Makefile for CSF Assignment 3


FLAGS = -Wall -Wextra -pedantic

all : csim

csim : csim.o cache.o helper_functions.o
	g++ csim.o cache.o helper_functions.o -o csim

cache.o : cache.cpp cache.h
	g++ $(FLAGS) -c cache.cpp

helper_functions.o : helper_functions.cpp helper_functions.h
	g++ $(FLAGS) -c helper_functions.cpp

csim.o : csim.cpp
	g++ $(FLAGS) -c csim.cpp

clean :
	rm -f csim *.o