all: consola R Rp MM PM 

consola:
	gcc consola.o -o consola

R: R.o 
	gcc R.o -o R

Rp: Rp.o
	gcc Rp.o -o Rp

MM: MM.o 
	gcc MM.o -o MM

PM: PM.o
	gcc PM.o -o PM
	

consola.o: consola.c
	gcc -Wall -c consola.c

R.o: R.c 
	gcc -Wall -c R.c 

Rp.o: Rp.c
	gcc -Wall -c Rp.c

MM.o: MM.c
	gcc -Wall -c MM.c

PM.o: PM.c constantes.h
	gcc -Wall -c PM.c

sock.o: sock.c
	gcc -Wall -c sock.c	


clean:
	rm *.o
