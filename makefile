all: test clean

test: main.o netapi.o
	g++ -o test main.o netapi.o -lpthread -lncurses

main.o: main.cpp 
	g++ -c main.cpp -lncurses -std=c++0x -lpthread -o main.o
	
netapi.o: netapi.cpp netapi.hpp
	g++ -c netapi.cpp -std=c++0x -lpthread -o netapi.o

clean:
	rm -f *.o
	

