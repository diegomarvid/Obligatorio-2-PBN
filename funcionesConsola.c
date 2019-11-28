#include <error.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include "InputFormat.c"
#include "sock.c"
#include "constantes.h"
#include "consola.h"

#define MAX_PORT 65535

int monitored_fd_set[2] = {-1, -1};


void refresh_fd_set(fd_set *fd_set_ptr) {
    FD_ZERO(fd_set_ptr);
    FD_SET(monitored_fd_set[0], fd_set_ptr);
    FD_SET(monitored_fd_set[1], fd_set_ptr);
}



/*----------------------------INTERRUPCIONES---------------------------------*/
//Maneja la interrupcion sigInt
void sigIntHandler(int signum, siginfo_t *info, void *ucontext ) {

    printf("Cerrando consola... \n");


	exit(EXIT_SUCCESS);
}

//Seetings de la interrupcion sigInt
void sigIntSet(void) {
    struct sigaction action, oldaction;

    action.sa_sigaction = sigIntHandler; //Funcion a llamar
    sigemptyset(&action.sa_mask);
    sigfillset(&action.sa_mask); //Bloqueo todas la seniales
    action.sa_flags = SA_SIGINFO;
    action.sa_restorer = NULL;

    sigaction(SIGINT, &action, &oldaction);
}
/*----------------------------I------------------------------------------------*/



/*-----------------------------------------DESPLEGAR MENU-------------------------------------------------*/
//Imprime el menu en pantalla.
void desplegar_menu(void){

	printf("\n\n");

	printf("Las opciones del menu son:\n1-Crear proceso.\n2-Eliminar proceso.\n3-Suspender proceso.\n4-Reanudar proceso.");
	printf("\n5-Ver. estado proceso.\n6-Ver lista de procesos.\n7-Cerrar consola\n8-Cerrar sistema.\n9-Ver salida proceso\nIngrese un numero del ");
	printf("1 al 9\n\n");

	return;
}



/*-----------------------------------CREAR MENSAJE-------------------------------------*/
//Dada la opcion ingresada, pide los datos necesarios para realizar la mimsa
// y genera el mensaje listo para ser enviado a Rp.
int crear_mensaje(int opcion, char msg[]){

	//Informacion sobre la operacion a realizar
	char data[CMD_SIZE];
	int PID;


	switch (opcion){

		case 1:

			readCMD(data);

			sprintf(msg, "%d-%s", opcion, data);

			break;

		case 2:

			PID = readPID("Ingrese el pid del proceso a eliminar.");

			sprintf(data,"%d",PID);

			sprintf(msg, "%d-%s", opcion, data);

			break;
		case 3:

			PID = readPID("Ingrese el pid del proceso a suspender.");

			sprintf(data,"%d",PID);

			sprintf(msg, "%d-%s", opcion, data);

			break;
		case 4:

			PID = readPID("Ingrese el pid del proceso a reanudar.");

			sprintf(data,"%d",PID);

			sprintf(msg, "%d-%s", opcion, data);

			break;
		case 5:

			PID = readPID("Ingrese el pid del proceso a ver estado.");

			sprintf(data,"%d",PID);

			sprintf(msg, "%d-%s", opcion, data);

			break;
		case 6:

			strcpy(data , "Listar procesos.");

			sprintf(msg, "%d-%s", opcion, data);

			break;
		case 8:

			strcpy(data , "Eliminar sistema.");

			sprintf(msg, "%d-%s", opcion, data);

			break;
		case 9:

			PID = readPID("Ingrese el pid del proceso a ver.");

			sprintf(data, "%d", PID);

			sprintf(msg, "%d-%s", opcion, data);

			break;

		default:
			printf("Error, ingreso erroeneo.");

	}

	return 0;

}

/*--------------------MENSAJES A SERVIDOR SINCRONOS---------------------------*/
//Esta función envia una tarea a realizar por el servidor y espera hasta que el servidor responda su resultado.
void transimitir_mensaje(int sockfd, char mensaje[], char respuesta[]) {



	/*

	Cada operacion tiene una respuesta sincronica,
	entre medio pueden llegar resultados asincronicos
	pero el servidor debe esperar y no desplegar el
	menu hasta recibir la respuesta asociada a la
	operacion realizada por el usuario. Es por esto
	que se utiliza un while donde se sigue leyendo
	hasta que se reciba una respuesta sincronica.

	*/

	int comunicacion;
	int r;

	//Envia el mensaje con formato asociado.

	if (send(sockfd, mensaje, (strlen(mensaje) + 1), MSG_NOSIGNAL) < 0)
	{
		MYERR(EXIT_FAILURE, "[C] Error en el send \n");
	}

	printf("[C]->[Rp] Manda: %s \n", mensaje);


	//Recibe mensajes del servidor hasta que llegue el resultado de la tarea.
	do{

		r = recv(sockfd, respuesta, RESPUESTA_BUFFSIZE, 0);

		if ( r == ERROR_CONNECTION){
			MYERR(EXIT_FAILURE, "[C] Se cayo el servidor \n");
		} else if(r == END_OF_CONNECTION){
			MYERR(EXIT_FAILURE, "[C] Conexion finalizada con el servidor \n");
		}

		//Los mensajes del servidor son de la forma comunicacion-resultado.

		//Separo el mensajes en sus dos componentes.
		sscanf(respuesta,"%d-%[^'\n']s",&comunicacion,respuesta);

		//Cambio char '27' a enter para imprimirlo en un formato visible
		 replace_char(respuesta, (char) 27, '\n');

		//Si es la respuesta de la operacion la muestro en pantalla con su formato.
		if(comunicacion == SINCRONICO){
			printf("[Rp]->[C] Recibe: %s\n", respuesta);
		}
		//Si es errores en procesos creados por la terminal lo muestro en pantalla con su formato.
		else if (comunicacion == ASINCRONICO){
			printf("|ADVERTENCIA|: %s\n", respuesta);
		}


	//Leo los mensajes del servidor hasta obtener la respuesta de la operación.
	} while(comunicacion != SINCRONICO);

}
