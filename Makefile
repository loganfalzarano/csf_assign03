CC = g++

CFLAGS = -g -Wall -Wextra -pedantic -std=gnu++11 -O2

%.o : %.c
	$(CC) $(CFLAGS) -c $*.c -o $*.o

all : csim

csim : csim.o
	$(CC) -o $@ csim.o

csim.o : csim.cpp
	$(CC) $(CFLAGS) -c $*.cpp -o $*.o

clean :
	rm -f fixedpoint_tests *.o