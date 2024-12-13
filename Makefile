CC = g++
CFLAGS = -std=c++11 -Wall -lpthread

SRC = FuncA.cpp HTTPServer.cpp
OBJ = $(SRC:.cpp=.o)
EXEC = http_server

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $(EXEC)

.cpp.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(EXEC)
