CC=gcc
IDIR=../include

CFLAGS=-O2 -Wall
CFLAGS += -I$(IDIR)

#OBJ = atca-ioc-int-lib.o
#DEPS = atca-ioc-int-lib.h common.h
LDFLAGS =    -lm

default: testTiming

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) -I$(IDIR)

testDMA: $(OBJ) testDMA.o
	$(CC) -o $@ $^ $(CFLAGS) -I$(IDIR)

testTiming: $(OBJ) testTiming.o
	$(CC) -o $@ $^ $(CFLAGS) -I$(IDIR)

clean:
	rm -f  *.o *~
