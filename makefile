###########################################################################
 ## Program Filename: makefile
 ## Author: Sara Hashem, Matthew Palmer, Niza Volair
 ## Date: 08/06/16
 ##########################################################################

# Standard variables
CXX = g++
CXXFLAGS = -std=c++11
CXXFLAGS += -g

all:
	g++ ${CXXFLAGS} main.cpp -o tsp
	
clean:
	rm tsp