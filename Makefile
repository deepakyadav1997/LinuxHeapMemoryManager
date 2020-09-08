target:exe
testapp.o:testapp.c
	gcc -g -c testapp.c -o testapp.o
mm.o:mm.c
	gcc -g -c mm.c -o mm.o
exe:mm.o testapp.o
	gcc -g testapp.c mm.o -o exe

clean:
	rm -f *.o
	rm -f exe
	

	 
	 
	 