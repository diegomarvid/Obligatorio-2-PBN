output: R.o Rp.o MM.o
	
R.o: R.c 
	gcc -Wall R.c -o R

Rp.o: Rp.c
	gcc -Wall Rp.c -o Rp

MM.o: MM.c
	gcc -Wall MM.c -o MM	

clean:
	rm *.o
