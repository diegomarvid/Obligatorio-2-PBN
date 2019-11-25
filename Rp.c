#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <error.h>
#include <errno.h>
#include "sock.c"
#include "constantes.h"
#include "Rp.h"


int monitored_fd_set[2] = {-1, -1};

//Recibe un mensaje como un string y lo transforma en una estructura interna Mensaje.
void conv_to_struct(Mensaje *mensaje, char buffer[]){

    int op;
    char data[CMD_SIZE];

    //Lee hasta el enter del cmd
    if(sscanf(buffer, "%d-%[^\n]s", &op, data) == 0) {
        MYERR(EXIT_FAILURE, "Error en la conversion a mensaje");
    }

    mensaje->id = RP;
    mensaje->RID = getpid();
    mensaje->op = op;
    strcpy(mensaje->data, data);

}


void refresh_fd_set(fd_set *fd_set_ptr) {
    FD_ZERO(fd_set_ptr);
    FD_SET(monitored_fd_set[0], fd_set_ptr);
    FD_SET(monitored_fd_set[1], fd_set_ptr);
}




// void tarea_consola(int consola_socket, int mm_socket ,char[] buffer, Mensaje mensaje){

//                read = recv(consola_socket, buffer, BUFFSIZE, 0);

//             if (read < 0)
//             {

//                 close(consola_socket);
//                 close(mm_socket);

//                 MYERR(EXIT_FAILURE, "[Rp] Error en el recv \n");

//             }
//             else if (read == 0)
//             {

//                 close(consola_socket);
//                 close(mm_socket);

//                 MYERR(EXIT_FAILURE, "[Rp] Conexion finalizada \n");
//             }

//             printf("[Rp] Recibe de consola: %s \n", buffer);

//             //CONVIERTO A ESTRUCTURA INTERNA DEL SERVIDOR

//             conv_to_struct(&mensaje, buffer);

//             //*********MANDA A MM***********//

//             if (send(mm_socket, &mensaje, sizeof(mensaje), MSG_NOSIGNAL) <= 0)
//             {

//                 close(mm_socket);

//                 MYERR(EXIT_FAILURE, "[Rp] Error en el send \n");

//             }
// }



int main(int argc, char const *argv[])
{

    //Creo socket
    //int socket = sock_listen(PORT);

    char buffer[ENTRADA_BUFFSIZE];
    char respuesta[RESPUESTA_BUFFSIZE];

    printf("[Rp] Se creo un socket entre en el puerto: %d \n", PORT);


    //Obtengo el valor del socket que conecta a Rp con su consola.
    int consola_socket = atoi(argv[1]);
    //Variable que guarda el socket que conecta a Rp con MM.
    int mm_socket;

    //Inicializo la estructura mensaje con valores default.
    Mensaje mensaje = {getpid(), -1, "", RP};

    //Conecto a Rp con MM.
    mm_socket = sock_connect_un();

    if( mm_socket == 0 ){
        MYERR(EXIT_FAILURE, "MM se desconecto \n");
    } else if( mm_socket < 0) {
        MYERR(EXIT_FAILURE, "Error, no se pudo aceptar conexion. \n");
    }


    printf("Connection established with MM \n");

    //*****Variables select*******//

    //Maximo fd para el parametro del select
    int max_fd = -1;

    //Guardo fd en el array para el select
    monitored_fd_set[0] = consola_socket;
    monitored_fd_set[1] = mm_socket;

    //Calculo del maximo fd
    if(consola_socket > mm_socket) {
        max_fd = consola_socket;
    } else {
        max_fd = mm_socket;
    }

    fd_set readfds;

    //******Variables socket******//
    int read;

    while(TRUE) {

        //Actualizo los fd a controlar por el select.
        refresh_fd_set(&readfds);

        //Realizo un select el cual nos permite saber si usuario desea realizar una accion o MM nos envia resultados.
        select(max_fd + 1, &readfds, NULL, NULL, NULL);

        if(FD_ISSET(consola_socket, &readfds)) {

        //    if( tarea_consola(consola_socket,mm_socket ,buffer, mensaje)==EXIT_FAILURE){

        //        return EXIT_FAILURE;
        //    }
            //*********RECIBE CONSOLA********//

            read = recv(consola_socket, buffer, ENTRADA_BUFFSIZE, 0);

            if (read == ERROR_CONNECTION){
                close(consola_socket);
                close(mm_socket);
                MYERR(EXIT_FAILURE, "[Rp] Error en el recv \n");
            }
            else if (read == END_OF_CONNECTION){
                close(consola_socket);
                close(mm_socket);
                MYERR(EXIT_FAILURE, "[Rp] Conexion finalizada con Consola\n");
            }

            printf("[Rp] Recibe de consola: %s \n", buffer);

            //CONVIERTO A ESTRUCTURA INTERNA DEL SERVIDOR

            conv_to_struct(&mensaje, buffer);

            //*********MANDA A MM***********//

            if (send(mm_socket, &mensaje, sizeof(mensaje), MSG_NOSIGNAL) <= 0){
                close(mm_socket);
                MYERR(EXIT_FAILURE, "[Rp] Error en el send \n");
            }

        } else if(FD_ISSET(mm_socket, &readfds)) {

            //*********RECIBE DE MM***********//
            //Rebice mensaje y lo formatea.

            read = recv(mm_socket, &mensaje, sizeof(mensaje), 0);

            if (read == ERROR_CONNECTION){
                close(mm_socket);
                MYERR(EXIT_FAILURE, "[Rp] Error en el recv \n");
            }
            else if (read == END_OF_CONNECTION){
                close(mm_socket);
                MYERR(EXIT_FAILURE, "[Rp] Conexion finalizada con MM \n");
            }

            printf("[Rp] Recibe de MM: %s con id %d \n", mensaje.data, mensaje.id);


            //*********MANDA A Consola***********//
            //Formatear mensaje para consola.

            //A Rp le llega de la forma PID-RESPUESTA_STR
            strcpy(respuesta, "");

            if(mensaje.id == MM){
                sprintf(respuesta, "%d-%s", SINCRONICO, mensaje.data);
            }else if (mensaje.id == PM){
                sprintf(respuesta, "%d-%s", ASINCRONICO, mensaje.data);
            }

            //printf("%s\n",buffer);

            if (send(consola_socket, respuesta, strlen(respuesta) + 1, MSG_NOSIGNAL) <= 0){
                close(consola_socket);
                MYERR(EXIT_FAILURE, "[Rp] Error en el send \n");
            }

            printf("[Rp]->[C] Manda: %s \n", respuesta);
        }

   }

    return 0;
}
