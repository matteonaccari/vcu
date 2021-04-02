CXXDEBUG=-g -O0
CXXRELEASE= -g -O3
LDDEBUG=-g
CXXFLAGS=-std=c++11 -D_FILE_OFFSET_BITS=64
CC=g++

all: debug release

debug: main.o
	$(CC) $(CXXDEBUG)  -o spatiotemporaltndex-debug main.o

release: main.o
	$(CC) $(CXXRELEASE) -o spatiotemporalindex main.o

main.o: spatialtemporalindex.h main.cpp
	$(CC) $(CXXFLAGS) -g -c main.cpp

clean:
	-@rm *.o spatiotemporalindex spatiotemporaltndex-debug 2> /dev/null || true
