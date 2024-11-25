AUTOMAKE_OPTIONS = foreign

bin_PROGRAMS = myprogram
myprogram_SOURCES = main.cpp FuncA.cpp FuncA.h

check_PROGRAMS = test_program
test_program_SOURCES = test_func_a.cpp FuncA.cpp FuncA.h

test_program_LDADD = -lgtest -lgtest_main -pthread

dist_doc_DATA = README.md
