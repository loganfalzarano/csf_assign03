CC = g++

# Note: we use -std=gnu11 rather than -std=c11 in order to use the
# sigjmp_buf data type
CFLAGS = -g -Wall -Wextra -pedantic -std=gnu11

%.o : %.c
	$(CC) $(CFLAGS) -c $*.c -o $*.o

all : csim

csim : csim.o
	$(CC) -o $@ csim.o

csim.o : csim.cpp 

clean :
	rm -f fixedpoint_tests *.o