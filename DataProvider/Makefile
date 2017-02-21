CC=clang++
CFLAGS=-std=c++14 -Wall -fPIC -c 
LFLAGS=-L../build/lib -lTextFileProvider -shared -Wl,-rpath=$$ORIGIN/../lib/
SRC=$(wildcard *.cpp)
OBJ=$(addprefix obj/,$(notdir $(SRC:.cpp=.o)))
LIB=../build/lib

all: libDataProvider.so

libDataProvider.so: $(OBJ) 
	$(CC) $^ $(LFLAGS) -o $(LIB)/$@

$(OBJ): $(SRC) | obj
	$(CC) $(CFLAGS) $< -o $@

obj:
	mkdir obj

clean:
	rm $(OBJ) $(LIB)/libDataProvider.so
