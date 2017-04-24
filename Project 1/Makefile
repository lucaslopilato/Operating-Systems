CC      = g++
CFLAGS  = -O 
LDFLAGS  = -O -lpthread 

FLAGS0  = -O -lpthread -DHASHCHAIN
FLAGS1  = -O -lpthread 
FLAGS2  = -O -lpthread -DRWLOCK 
FLAGS3  = -O -lpthread -DFINEGRAIN
FLAGS4  = -O -lpthread -DFINEGRAIN -DRWLOCK 

all:  phashchain phashcoarse phashcoarserw  phashfine phashfinerw
#all:  hashchain phashchain phashcoarse phashcoarserw  phashfine phashfinerw


phashchain: hashchain.o ptest.cc
	$(CC) -o $@ $^ $(FLAGS0)

phashcoarse: phash.cc rwlock.cc ptest.cc
	$(CC) -o $@ $^ $(FLAGS1)

phashcoarserw: phash.cc rwlock.cc ptest.cc
	$(CC) -o $@ $^ $(FLAGS2)

phashfine: phash1.cc rwlock.cc ptest.cc
	$(CC) -o $@ $^ $(FLAGS3)

phashfinerw: phash1.cc rwlock.cc ptest.cc
	$(CC) -o $@ $^ $(FLAGS4)

run: 
	./phash

run1: 
	./phashcoarse

run2: 
	./phashcoarserw

run3: 
	./phashfine

run4: 
	./phashfinerw

.cc.o: 
	$(CC)  $(CFLAGS) -c $<


clean:
	rm *.o hashchain phashchain phashcoarse phashcoarserw  phashfine phashfinerw

