target:exe
testapp.o:testapp.c
	gcc -g -c testapp.c -o testapp.o
mm.o:mm.c
	gcc -g -c mm.c -o mm.o
glthread.o:glthreads_lib/glthread.c
	gcc -g -c glthreads_lib/glthread.c -o glthread.o
exe:mm.o testapp.o glthread.o
	gcc -g testapp.c mm.o glthread.o -o exe

clean:
	rm -f *.o
	rm -f exe
	

	 
	 
	 