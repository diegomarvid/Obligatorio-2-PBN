#include <stdio.h>
#include "InputFormat.h"


void desplegar_menu(){

	printf("Las opciones del menu son:\n1-Crear proceso.\n2-Eliminar proceso.\n3-Suspender proceso.\n4-Reanudar proceso.");
	printf("\n5-Ver estado proceso.\n6-Ver lista de procesos.\n7-Cerrar consola\n8-Cerrar sistema.\n");

	return;
}


int realizar_opcion(int opcion){

	switch (opcion){
	
		case 1:

		break;
		default:	
	
	}

	return 0;


}





int main(int argc,char *argv[]){

	int opcion;
	int puerto;
//	char*[] ip;
	int r;

	if (argc!=2){
		
		printf("Error, consola precisa mas datos.\n");

	}else{

		puerto=argv[0];

		IP=argv[1];
		
		//r=conect(puerto, IP);
        r=1;

		if (r==0){
		
			printf("Error de coexion, inente mas tarde.\n");

		}else{
			
			do{

				desplegar_menu();

				opcion = obtener_opcion(1,7);

				realizar_opcion(opcion);
			
			}while ( opcion != 7 );
			
			
			printf("Hasta luego!\n");
		}

	}

}