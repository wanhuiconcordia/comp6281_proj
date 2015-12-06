INCLPATH +=/usr/include
CFLAGS += -std=c11
LIBS   = -lm
OBJECTS = zipf.o a3.o

a3: $(OBJECTS)
	mpicc $(OBJECTS) $(LIBS) -o a3

zipf.o: zipf.c zipf.h
	mpicc $(CFLAGS) -o zipf.o -c zipf.c -I$(INCLPATH)
a3.o: a3.c
	gcc $(CFLAGS) -o a3.o -c a3.c -I$(INCLPATH)
clean:
	rm *.o

