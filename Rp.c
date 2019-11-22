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




void conv_to_struct(Mensaje *mensaje, char buffer[]){

    int op;
    char data[CMD_SIZE];

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



int main(int argc, char const *argv[])
{
    
    //Creo socket
    //int socket = sock_listen(PORT);

    char buffer[BUFFSIZE];

    char string[BUFFSIZE] = "Mensaje del servidor papu";

    printf("[Rp] Se creo un socket entre en el puerto: %d \n", PORT);

    int consola_socket = atoi(argv[1]);

    Mensaje mensaje = {getpid(), -1, "", RP};

    int mm_socket;

    mm_socket = sock_connect_un();

    if( mm_socket < 0 ){

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

        refresh_fd_set(&readfds);

        select(max_fd + 1, &readfds, NULL, NULL, NULL);

        if(FD_ISSET(consola_socket, &readfds)) {

            //*********RECIBE CONSOLA********//

            read = recv(consola_socket, buffer, BUFFSIZE, 0);

            if (read < 0)
            {

                close(consola_socket);

                MYERR(EXIT_FAILURE, "[Rp] Error en el recv \n");
            }
            else if (read == 0)
            {

                close(consola_socket);
                
                MYERR(EXIT_FAILURE, "[Rp] Conexion finalizada \n");
            }

            printf("[Rp] Recibe de consola: %s \n", buffer);

            //CONVIERTO A ESTRUCTURA INTERNA DEL SERVIDOR

            conv_to_struct(&mensaje, buffer);

            //*********MANDA A MM***********//

            if (send(mm_socket, &mensaje, sizeof(mensaje), MSG_NOSIGNAL) <= 0)
            {

                close(mm_socket);

                MYERR(EXIT_FAILURE, "[Rp] Error en el send \n");
            }

            printf("[Rp]->[MM] Manda mensaje \n");
            printf("RID: %d \n", mensaje.RID);
            printf("Op: %d \n", mensaje.op);
            printf("Data: %s \n", mensaje.data);
            printf("Id: %d \n", mensaje.id);

        } else if(FD_ISSET(mm_socket, &readfds)) {

            //*********RECIBE DE MM***********//

            read = recv(mm_socket, buffer, BUFFSIZE, 0);

            if (read < 0)
            {
                close(mm_socket);

                MYERR(EXIT_FAILURE, "[Rp] Error en el recv \n");
            }
            else if (read == 0)
            {

                close(mm_socket);

                MYERR(EXIT_FAILURE, "[Rp] Conexion finalizada \n");
            }

            printf("[Rp] Recibe de MM: %s \n", buffer);

            //*********MANDA A Consola***********//

            if (send(consola_socket, buffer, strlen(buffer) + 1, MSG_NOSIGNAL) <= 0)
            {

                close(consola_socket);

                MYERR(EXIT_FAILURE, "[Rp] Error en el send \n");
            }

            printf("[Rp]->[C] Manda: %s \n", buffer);
        }
        

        

       

       
       

   }


    return 0;
}
