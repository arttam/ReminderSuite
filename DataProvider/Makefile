CC=clang++
CFLAGS=-std=c++14 -Wall -c -fPIC
LFLAGS=-shared
SRC=$(wildcard src/*.cpp)
OBJ=$(addprefix obj/,$(notdir $(SRC:.cpp=.o)))
LIB=../build/lib

all: libDataProvider.so

libDataProvider.so: $(OBJ)
	$(CC) $^ $(LFLAGS) -o $(LIB)/$@

obj/%.o: src/%.cpp
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm $(OBJ) $(LIB)/libDataProvider.so

