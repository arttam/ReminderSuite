CC=clang++
CFLAGS=-std=c++14 -Wall -c
LFLAGS=-L../build/lib -lboost_system -lpthread -ldl -lRDBClient -Wl,-rpath='$$ORIGIN/../lib/'
SRC=$(wildcard src/*.cpp)
OBJ=$(addprefix obj/,$(notdir $(SRC:.cpp=.o)))
BIN=../build/bin

all: reminder-http

reminder-http: $(OBJ)
	$(CC) $^ $(LFLAGS) -o $(BIN)/$@

obj/%.o: src/%.cpp | obj
	$(CC) $(CFLAGS) $< -o $@

obj:
	mkdir obj

clean:
	rm $(OBJ) $(BIN)/reminder-http

Usage: http_server <address> <port> <doc_root>
run: reminder-http
	$(BIN)/reminder-http 127.0.0.1 9001 /var/tmp/wasm/rdb
