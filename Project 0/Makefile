CC      = gcc
CFLAGS  = -O
LDFLAGS  = -O 


all: simple 

simple:  simple_shell.o
	$(CC) -o $@ $^ $(LDFLAGS)

run: 
	./simple

test: 
	./simple < testfile

clean:
	rm simple


.c.o:
	$(CC)  $(CFLAGS) -c $<

