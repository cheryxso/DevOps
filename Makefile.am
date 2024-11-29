AUTOMAKE_OPTIONS = foreign

bin_PROGRAMS = myprogram
myprogram_SOURCES = main.cpp FuncA.cpp FuncA.h

check_PROGRAMS = testprogram
testprogram_SOURCES = test_FuncA.cpp FuncA.cpp FuncA.h

testprogram_LDFLAGS = -pthread
testprogram_LDADD = -lgtest -lgtest_main

test_FuncA.o: test_FuncA.cpp FuncA.h
    $(CXX) $(CXXFLAGS) -c $< -o $@
