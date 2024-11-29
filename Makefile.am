AUTOMAKE_OPTIONS = foreign

bin_PROGRAMS = myprogram
myprogram_SOURCES = main.cpp FuncA.cpp FuncA.h

check_PROGRAMS = test_program
test_program_SOURCES = tests/test_FuncA.cpp FuncA.cpp FuncA.h

test_program_LDFLAGS = -pthread
test_program_LDADD = -lgtest -lgtest_main

tests/test_FuncA.o: tests/test_FuncA.cpp FuncA.h
<TAB>$(CXX) $(CXXFLAGS) -c $< -o $@
