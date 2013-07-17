beetle:	beetle.o child.o child.h
	gcc beetle.o child.o child.h -lm -o beetle

child.o: child.c child.h

clean:
	rm -f core *.o beetle
