AUTOMAKE_OPTIONS = foreign

bin_PROGRAMS = myprogram
myprogram_SOURCES = main.cpp FuncA.cpp FuncA.h

check_PROGRAMS = test_program
test_program_SOURCES = test_FuncA.cpp FuncA.cpp FuncA.h

test_program_LDFLAGS = -pthread
test_program_LDADD = -lgtest -lgtest_main
