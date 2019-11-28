#include "funcionesConsola.c"


/*-------------------------------------------------------------MAIN----------------------------------------------------------------------*/

int main(int argc,char *argv[]){

	//Configuro las interrupciones.
	sigIntSet();

	//Variables auxiliares necesarias para el funcionamiento del menu.
	int opcion = 1;
	char mensaje[ENTRADA_BUFFSIZE];
	char respuesta[RESPUESTA_BUFFSIZE];


	//Conecto con el servidor en cuestion y almaceno su socket asociado.
	int socket = sock_connect_in(SERVERHOST, PORT);

	//Variable utilizada en la seccion de mensajes asincronicos del servidor.
	int comunicacion;

	//Variables select

	//Guardo fd en el array para el select, los unicos son el socket con el servidor y STDIN.
	monitored_fd_set[0] = STDIN_FILENO;
	monitored_fd_set[1] = socket;

	//Set para el select
	fd_set readfds;

	//Variable para read en el socket
	int r;

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
		if (FD_ISSET(STDIN_FILENO, &readfds))
		{

			//Obtengo la opcion que el desea.
			opcion = readInt(1, 9);

			//Mando a realizar tal tarea y espero su resultado.
			if (opcion != 7)
			{
				crear_mensaje(opcion, mensaje);
				transimitir_mensaje(socket, mensaje, respuesta);
			}

			//Luego de realizar una tarea y obtener su resultado, nuevamente despliego el menu.
			desplegar_menu();

			//Si el servidor envia mensaje de procesos fallidos la consola los capta mientras que no se este realizando ninguna operacion.
		}
		else if (FD_ISSET(socket, &readfds))
		{

			memset(respuesta, 0, RESPUESTA_BUFFSIZE);

			//Lee del servidor y formatea su mensaje.
			r = recv(socket, respuesta, RESPUESTA_BUFFSIZE, 0);

			//printf("Cantidad de bytes recibidos: %d\n", r);

			if (r == ERROR_CONNECTION)
			{
				shutdown(socket, SHUT_RDWR);
				close(socket);
				MYERR(EXIT_FAILURE, "Se cayo el servidor.");
			}
			else if (r == END_OF_CONNECTION)
			{
				shutdown(socket, SHUT_RDWR);
				close(socket);
				MYERR(EXIT_FAILURE, "Se termino la conexion con el servidor");
			}

			//printf("%s",respuesta);

			sscanf(respuesta, "%d-%[^'\v']s", &comunicacion, respuesta);

			//Evaluar largo por que al principio y fin de conexion la pipe manda null.
			if (strlen(respuesta) > 0)
			{
				printf("%s", respuesta);
			}
		}

	} while (opcion != 7);

	//Cierre de consola, se envio el mensaje de cierre, solo hace falta cerrar su socket.
	close(socket);

	printf("Hasta luego!\n");

	return 0;
}
