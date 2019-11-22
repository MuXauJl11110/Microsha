CFLAGS=-c -std=c++1z -O3 # Compilation flags
LFLAGS=-fsanitize=address -g -lstdc++fs # Linker flags
CC=g++ # Compiler

HEADERS=shell.h # Headers

all: microsha

microsha: main.o shell.o
	$(CC) main.o shell.o $(LFLAGS) -o microsha

main.o: main.cpp $(HEADERS)
	$(CC) $(CFLAGS) main.cpp

shell.o: shell.cpp $(HEADERS)
	$(CC) $(CFLAGS) shell.cpp

clean:
	rm -rf *.o microsha
