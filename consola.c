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
void sigIntSet() {
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
void desplegar_menu(){

	printf("Las opciones del menu son:\n1-Crear proceso.\n2-Eliminar proceso.\n3-Suspender proceso.\n4-Reanudar proceso."); 
	printf("\n5-Ver. estado proceso.\n6-Ver lista de procesos.\n7-Cerrar consola\n8-Cerrar sistema.\nIngrese un numero del ");
	printf("uno al 8\n");

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
		default:	
			printf("Error, ingreso erroeneo.");
	
	}

	return 0;

}

/*--------------------MENSAJES A SERVIDOR SINCRONOS---------------------------*/
//Esta función envia una tarea a realizar por el servidor y espera hasta que el servidor responda su resultado.
void transimitir_mensaje(int sockfd, char mensaje[], char respuesta[], int emisor) {
	

	//Envia el mensaje con formato asociado.

	if (send(sockfd, mensaje, (strlen(mensaje) + 1), MSG_NOSIGNAL) < 0)
	{
		MYERR(EXIT_FAILURE, "[C] Error en el send \n");
	}

	printf("[C] Manda: %s \n", mensaje);


	//Recibe mensajes del servidor hasta que llegue el resultado de la tarea.
	do{

		if (recv(sockfd, respuesta, BUFFSIZE, 0) < 0)
		{
			MYERR(EXIT_FAILURE, "[C] Error en el recv \n");
		}

		//Los mensajes del servidor son de la forma emisor-resultado.

		//Separo el mensajes en sus dos componentes.
		sscanf(respuesta,"%d-%[^'\n']s",&emisor,respuesta);

		//Si es la respuesta de la operacion la muestro en pantalla con su formato.
		if(emisor== MM){

			printf("[C]Recibe: %s\n", respuesta);

		}
		//Si es errores en procesos creados por la terminal lo muestro en pantalla con su formato.
		else if (emisor == PM)
		{
			
			printf("Respuesta: %s\n", respuesta);

		}
		

	//Leo los mensajes del servidor hasta obtener la respuetsa de la operación. 
	}while(emisor != MM);

}


/*-------------------------------------------------------------MAIN----------------------------------------------------------------------*/

int main(int argc,char *argv[]){

	//Configuro las interrupciones.
	sigIntSet();

	//Variables utilziadas para conectarse con esl servidor.
	unsigned int ip[4], ipport;
	char txt_ip[16];

	//Variables auxiliares necesarias para el funcionamiento del menu.
	int opcion = 1;
	char mensaje[BUFFSIZE];
	char respuesta[BUFFSIZE];
	//char cmd[CMD_SIZE];

	
	//-----------Obtengo PORT e IP para conectarse----------//

	//do{

		// printf("Ingrese la direccion del servidor (ddd.ddd.ddd.ddd,pppp):\n");
		
		// while (scanf("%u.%u.%u.%u,%u", ip, ip+1, ip+2, ip+3, &ipport) != 5 || ip[0] > 255 || ip[1] > 255 || ip[2] > 255 || ip[3] > 255 || ipport > MAX_PORT || ipport < 3000) {
			
		// 	//Limpio stdin y pido nuevamente.
		// 	while ( getchar() != '\n' );

		// 	MYERR(EXIT_SUCCESS, "Error al ingresar direccion ip y/o el puerto, ingreselo nuevamente:");
		// }

		// sprintf(txt_ip, "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);


		// int socket = sock_connect(txt_ip, ipport);



		
		//Conecto con el servidor en cuestion y almaceno su socket asociado.
		int socket = sock_connect_in(SERVERHOST, PORT);

		//Variable utilizada en la seccion de mensajes asincronicos del servidor.
		int emisor;

		//******Variables select*******//
		//Guardo fd en el array para el select, los unicos son el socket con el servidor y STDIN.
		monitored_fd_set[0] = STDIN_FILENO;
		monitored_fd_set[1] = socket;

		fd_set readfds;

		//Antes de comenzar se imprime el menu para asegurar su primera iteracion.
		desplegar_menu();

		//codigo d ela consola en su funcionamiento normal.
		do
		{
			
			//Actualizo los fd a controlar por el select.
			refresh_fd_set(&readfds);
			//Realizo un select el cual nos permite saber si el usuario desea realizar una accion o no.
			select(socket + 1, &readfds, NULL, NULL, NULL);


			//Si el usuario desea realizar una tarea.
			if(FD_ISSET(STDIN_FILENO, &readfds)) {

				//Obtengo la opcion que el desea.
				opcion = readInt(1, 8);

				//Mando a realizar tal tarea y espero su resultado.
				if (opcion != 7) {
					crear_mensaje(opcion, mensaje);
					transimitir_mensaje(socket, mensaje, respuesta, emisor);
				}

			//Luego de realizar una tarea y obtener su resultado, nuevamente despliego el menu.	
			desplegar_menu();

			//Si el servidor envia mensaje de procesos fallidos la consola los capta mientras que no se este realizando ninguna operacion.	
			}else if(FD_ISSET(socket, &readfds)) {

				//Lee del servidor y formatea su mensaje.
				if(recv(socket, respuesta, BUFFSIZE, 0) <= 0) {
					MYERR(EXIT_FAILURE, "ERROR EN EL READ.");
				}

				sscanf(respuesta,"%d-%[^'\n']s",&emisor,respuesta);

				printf("\n Respuesta: %s \n", respuesta);

			}

			//desplegar_menu();
		
	    } while ( opcion != 7 );

		//Cierre de consola, se envio el mensaje de cierre, solo hace falta cerrar su socket.
		close(socket);
			
		printf("Hasta luego!\n");
		


	return 0;

	}

