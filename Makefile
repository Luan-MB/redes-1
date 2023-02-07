CC     = g++
CFLAGS = -Wall

PROG = client server
OBJS = raw_socket.o Mensagem.o

.PHONY: clean purge all

all: $(PROG)

$(PROG): %: $(OBJS) %.o
	$(CC) $(CFLAGS) -o  $@ $^

debug: CFLAGS += -DDEBUG -g
debug: all

clean:
	@rm -f *.o

purge:   
	@rm -f $(PROG)