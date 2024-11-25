CC = g++
CFLAGS = -std=c++11 -Wall

SRC = main.cpp FuncA.cpp
OBJ = $(SRC:.cpp=.o)
EXEC = myprogram

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $(EXEC)

.cpp.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(EXEC)
