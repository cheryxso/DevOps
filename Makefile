CC = g++
CFLAGS = -std=c++11 -Wall

SRC = main.cpp FuncA.cpp HttpServer.cpp
OBJ = $(SRC:.cpp=.o)
EXEC = ln_server

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $(EXEC)

.cpp.o:
	$(CC) $(CFLAGS) -c $< -o $@

run_server:
	./$(EXEC)

run_tests:
	@echo "Running manual tests..."
	@curl -i -X GET http://127.0.0.1:8080/compute

clean:
	rm -f $(OBJ) $(EXEC)
