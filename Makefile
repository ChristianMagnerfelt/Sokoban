all: all.o
	g++ -o Sokoban *.o

all.o:	*.h
	g++ -c -W -std=c++0x *.cc

clean:
	rm -f *.o Sokoban
