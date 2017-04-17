CC      = gcc
CFLAGS  = -O -g
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

leak:
	valgrind --leak-check=yes echo "ls | sort" | ./simple 


.c.o:
	$(CC)  $(CFLAGS) -c $<

