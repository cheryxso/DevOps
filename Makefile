CC = g++
CFLAGS = -std=c++11 -Wall

SRC = main.cpp FuncA.cpp HttpServer.cpp
OBJ = $(SRC:.cpp=.o)
EXEC = server

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $(EXEC)

clean:
	rm -f $(OBJ) $(EXEC)
