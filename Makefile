CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic

all: program

program: main.o FuncA.o
	$(CXX) $(CXXFLAGS) -o program main.o FuncA.o

main.o: main.cpp FuncA.h
	$(CXX) $(CXXFLAGS) -c main.cpp

FuncA.o: FuncA.cpp FuncA.h
	$(CXX) $(CXXFLAGS) -c FuncA.cpp

clean:
	rm -f *.o program
