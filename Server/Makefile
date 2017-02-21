CC=clang++
CFLAGS=-std=c++14 -Wall -c
LFLAGS=-lboost_system -lpthread -ldl -Wl,-rpath=$$ORIGIN/../lib/
SRC=$(wildcard src/*.cpp)
OBJ=$(addprefix obj/,$(notdir $(SRC:.cpp=.o)))
BIN=../build/bin

all: reminder-dp

reminder-dp: $(OBJ)
	$(CC) $^ $(LFLAGS) -o $(BIN)/$@

obj/%.o: src/%.cpp
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm $(OBJ) $(BIN)/reminder-dp

run: reminder-dp
	$(BIN)/reminder-dp
