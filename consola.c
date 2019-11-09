//#include "InputFormat.h"

#include <error.h>
#include <errno.h>
#include <unistd.h>
#include "InputFormat.c"
#include "sock.c"
#include "constantes.h"

#define MAX_PORT 65535

//#define MYERR(status, ...) error_at_line(status, errno, __FILE__, __LINE__, __VA_ARGS__)

void desplegar_menu(){

	printf("Las opciones del menu son:\n1-Crear proceso.\n2-Eliminar proceso.\n3-Suspender proceso.\n4-Reanudar proceso.");
	printf("\n5-Ver estado proceso.\n6-Ver lista de procesos.\n7-Cerrar consola\n8-Cerrar sistema.\n");

	return;
}


int realizar_opcion(int opcion){

	switch (opcion){
	
		case 1:

		printf("%d",readPID("Perrassssssssss"));

		break;
		case 2:
		printf("Opcion seleccionada: eliminar proceso.\n");

		printf("%d",readInt(1,8));

		break;
		case 3:
		printf("Opcion seleccionada: suspender proceso.\n");

		printf("%s",readCMD());

		break;
		case 4:
		printf("Opcion seleccionada: reanudar proceso.\n");

		break;
		case 5:
		printf("Opcion seleccionada: ver estado de proceso.\n");

		break;
		case 6:
		printf("Opcion seleccionada: crear proceso.\n");

		break;
		case 7:
		printf("Opcion seleccionada: crear proceso.\n");

		break;
		default:	
		printf("Error, ingreso fallido.");
	
	}

	return 0;


}





int main(int argc,char *argv[]){

	int opcion=7;
	unsigned int ip[4], ipport;
	char txt_ip[16];
	char mensaje[BUFFSIZE] = "Hola que tal";
	
	//do{

		printf("Ingrese la direccion del servidor (ddd.ddd.ddd.ddd,pppp):\n");
		
		while (scanf("%u.%u.%u.%u,%u", ip, ip+1, ip+2, ip+3, &ipport) != 5 || ip[0] > 255 || ip[1] > 255 || ip[2] > 255 || ip[3] > 255 || ipport > MAX_PORT || ipport < 3000) {
			
			//Limpio stdin y pido nuevamente.
			while ( getchar() != '\n' );

			MYERR(EXIT_SUCCESS, "Error al ingresar direccion ip y/o el puerto, ingreselo nuevamente:");
		}

		sprintf(txt_ip, "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);


		int socket = sock_connect(txt_ip, ipport);
		printf("socket:%d\n",socket);

	//}while (socket == 1);	


			
			do{

				desplegar_menu();

				opcion = readInt(1,7);

				realizar_opcion(opcion);

				if(send(socket, mensaje, (strlen(mensaje) + 1), MSG_NOSIGNAL) < 0) {
    			MYERR(EXIT_SUCCESS, "Error en el send \n");
				}

				printf("[Consola] Manda: %s \n", mensaje);

				if(recv(socket, mensaje, BUFFSIZE, 0) < 0) {
   				 MYERR(EXIT_SUCCESS, "Error en el recv \n");
				}

			printf("[Consola] Recibe: %s \n", mensaje);	
			
			}while ( opcion != 7 );
			
			close(socket);
			
			printf("Hasta luego!\n");
		


	return 0;

	}

