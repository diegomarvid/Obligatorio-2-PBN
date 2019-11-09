#include <stdio.h>  
#include <stdlib.h> 
#include <sys/types.h>
#include <string.h>
#include "InputFormat.h"

#define MAX_LENGTH 21

//Funcion encargada de limiar la standard input.
int clean_stdin(void)
{
    while (getchar()!='\n');
    return 1;
}


//Lee y retorna un PID.
//readPID(mensaje a imprimir al pedir PID).

pid_t readPID( char msg[] ){
    
	int PID = 0;  
    	
	int r;

	char enter;
    
	do
        {  

       	printf( "%s. Recuerde que un PID numero mayor a 0.\n" , msg );
        
		r = scanf( "%9d%c" , &PID , &enter );
	
		if( r == 0 || enter != '\n' || PID <= 0 ){

     		    printf("Error, la entrada no posee un formato de PID valido.\n");

		}


   	} while ( ( ( r != 2 || enter != '\n' ) && clean_stdin() ) || PID <= 0 );


	printf("Entrada exitosa, el PID ingresado es: %d.\n",PID);

	return  (pid_t) PID; 
}


//Lee y retorna un int entre un rango deseado.

int readInt( int min , int max ){
 
    	
	int r;
	
	int num;

	char enter;
    
	do
        {  
       		printf( "Ingrese un numero entre %d y %d:\n", min , max );
        
			r=scanf("%9d%c", &num , &enter);
	
			if( r == 0 || enter != '\n' || num < min || num > max ){

     		    printf("Error, la entrada no posee un formato valido o no esta dentro del rango habilitado.\n");

			}


   	} while ( ( (r != 2 || enter != '\n') && clean_stdin() ) || num < min || num > max);


	return num; 

}


//Lee y retorna un string.

char * readCMD(void){
	
	static char cmd[MAX_LENGTH];

	int lenght;

	int esmaslargo;

	char* r;

	do{

		esmaslargo=0;

		printf("Ingrese un comando a ejecutar:\n");

		r=fgets (cmd, MAX_LENGTH, stdin);

		if(r==NULL)	{

		printf("Error de lectura");

		}else{

			lenght=strlen(cmd);


			if ( (lenght == (MAX_LENGTH-1) ) && cmd[lenght-1] != '\n'	){

				printf("El comando es mas largo de lo esperada.\n");

				esmaslargo=1;

				clean_stdin();

			}else if (lenght==1){

				printf("La entrada vacia no es un comando valido.\n");

				//getchar()
			}

		}

	}while( r == NULL || esmaslargo || lenght == 1);

	//printf("%s\n",cmd);
	cmd[strcspn(cmd,"\n")]=0;

	return cmd;
}

