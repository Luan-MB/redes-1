CC     = g++
CFLAGS = -Wall #-Wshadow

PROG = client server
OBJS = raw_socket.cpp

.PHONY: clean purge all

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) $@.o -o  $@ $^

clean:
	@rm -f *.o

purge:   
	@rm -f $(PROG)