//#include "InputFormat.h"

#include <error.h>
#include <signal.h>
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


int crear_mensaje(int opcion, char cmd[]){



	switch (opcion){
	
		case 1:
			readCMD(cmd);
			break;

		case 2:
			printf("Opcion seleccionada: eliminar proceso.\n");

			printf("%d",readInt(1,8));

			break;
		case 3:
			printf("Opcion seleccionada: suspender proceso.\n");

			//printf("%s",readCMD());

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


void sigIntHandler(int signum, siginfo_t *info, void *ucontext ) {

    printf("Cerrando consola... \n");

	
	exit(EXIT_SUCCESS);
}

void sigIntSet() {
    struct sigaction action, oldaction;

    action.sa_sigaction = sigIntHandler; //Funcion a llamar
    sigemptyset(&action.sa_mask);
    sigfillset(&action.sa_mask); //Bloqueo todas la seniales
    action.sa_flags = SA_SIGINFO;
    action.sa_restorer = NULL;

    sigaction(SIGINT, &action, &oldaction);
}


void transimitir_mensaje(int sockfd, char mensaje[], char respuesta[]) {

	if (send(sockfd, mensaje, (strlen(mensaje) + 1), MSG_NOSIGNAL) < 0)
	{
		MYERR(EXIT_FAILURE, "[C] Error en el send \n");
	}

	printf("[C] Manda: %s \n", mensaje);

	if (recv(sockfd, respuesta, BUFFSIZE, 0) < 0)
	{
		MYERR(EXIT_FAILURE, "[C] Error en el recv \n");
	}

	printf("[C] Recibe: %s \n", respuesta); 
}




int main(int argc,char *argv[]){

	sigIntSet();

	int opcion = 7;
	unsigned int ip[4], ipport;
	char txt_ip[16];
	char mensaje[BUFFSIZE];
	char respuesta[BUFFSIZE];
	char cmd[CMD_SIZE];
	
	//do{

		// printf("Ingrese la direccion del servidor (ddd.ddd.ddd.ddd,pppp):\n");
		
		// while (scanf("%u.%u.%u.%u,%u", ip, ip+1, ip+2, ip+3, &ipport) != 5 || ip[0] > 255 || ip[1] > 255 || ip[2] > 255 || ip[3] > 255 || ipport > MAX_PORT || ipport < 3000) {
			
		// 	//Limpio stdin y pido nuevamente.
		// 	while ( getchar() != '\n' );

		// 	MYERR(EXIT_SUCCESS, "Error al ingresar direccion ip y/o el puerto, ingreselo nuevamente:");
		// }

		// sprintf(txt_ip, "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);


		// int socket = sock_connect(txt_ip, ipport);
		int socket = sock_connect(SERVERHOST, PORT);
		printf("socket:%d\n",socket);
		
			do{

				desplegar_menu();

				opcion = readInt(1,8);

				if (opcion != 7)
				{

					crear_mensaje(opcion, cmd);

					sprintf(mensaje, "%d-%s", opcion, cmd);

					transimitir_mensaje(socket, mensaje, respuesta);

				}

			} while ( opcion != 7 );
			
			close(socket);
			
			printf("Hasta luego!\n");
		


	return 0;

	}

