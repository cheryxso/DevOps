AUTOMAKE_OPTIONS = foreign

bin_PROGRAMS = ln_server
ln_server_SOURCES = main.cpp FuncA.cpp FuncA.h HttpServer.cpp HttpServer.h

check_PROGRAMS = test_program
test_program_SOURCES = tests/test_FuncA.cpp FuncA.cpp FuncA.h
test_program_LDFLAGS = -pthread
test_program_LDADD = -lgtest -lgtest_main

dist_man_MANS = ln_server.1
dist_pkgdata_DATA = data.txt

SUBDIRS = tests

CTRLF_DIR=$(CURDIR)/deb/DEBIAN
CTRLF_NAME=$(CTRLF_DIR)/control

.PHONY: deb debug all

deb:
	mkdir -p $(CTRLF_DIR)
	echo Package: ln_server > $(CTRLF_NAME)
	echo Version: 1.0.0 >> $(CTRLF_NAME)
	echo Architecture: all >> $(CTRLF_NAME)
	echo Maintainer: your_email@example.com >> $(CTRLF_NAME)
	echo -n "Description: " >> $(CTRLF_NAME)
	cat ln_server.1 >> $(CTRLF_NAME)
	make DESTDIR=$(CURDIR)/deb install

all: ln_server

ln_server: $(ln_server_SOURCES)
	$(CXX) $(CXXFLAGS) $(ln_server_SOURCES) -o ln_server

tests/test_FuncA.o: tests/test_FuncA.cpp FuncA.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

run_tests: all
	./ln_server &
	sleep 2  
	curl -i -X GET http://127.0.0.1:8080/compute
	kill $(shell pidof ln_server)

# Команда для запуску сервера
run_server:
	./ln_server
