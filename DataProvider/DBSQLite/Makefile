CC=clang++
CFLAGS=-std=c++14 -Wall -fPIC -c 
LFLAGS=-shared -lsqlite3
SRC=$(wildcard src/*.cpp)
OBJ=$(addprefix obj/,$(notdir $(SRC:.cpp=.o)))
LIB=../../build/lib

libDBSQLiteProvider.so: $(OBJ)
	$(CC) $^ $(LFLAGS) -o $(LIB)/$@

obj/%.o: src/%.cpp | obj
	$(CC) $(CFLAGS) $< -o $@

obj:
	mkdir obj

clean:
	rm $(OBJ) $(LIB)/libDBSQLiteProvider.so
