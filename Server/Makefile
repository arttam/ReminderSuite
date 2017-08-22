CC=clang++
CFLAGS=-std=c++14 -Wall -c
LFLAGS=-lboost_system -lpthread -lboost_program_options -ldl -Wl,-rpath=$$ORIGIN/../lib/
SRC=$(wildcard src/*.cpp)
OBJ=$(addprefix obj/,$(notdir $(SRC:.cpp=.o)))
BIN=../build/bin

all: reminder-dp

reminder-dp: $(OBJ)
	$(CC) $^ $(LFLAGS) -o $(BIN)/$@

obj/%.o: src/%.cpp | obj
	$(CC) $(CFLAGS) $< -o $@

obj:
	mkdir obj

clean:
	rm $(OBJ) $(BIN)/reminder-dp

run: reminder-dp
	$(BIN)/reminder-dp
