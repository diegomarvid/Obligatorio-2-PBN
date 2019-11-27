#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <error.h>
#include <errno.h>
#include <fcntl.h> // para el mkfifo
#include <sys/stat.h> // para el mkfifo
#include <sys/types.h>// para el mkfifo
#include "sock.c"
#include "constantes.h"
#include "Rp.h"


int monitored_fd_set[3] = {-1, -1, -1};

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
    FD_SET(monitored_fd_set[2], fd_set_ptr);
}

int
get_max_fd(){

    int i;
    int max = -1;

    for(i = 0 ; i < 3; i++ ){

        if(monitored_fd_set[i] > max){

            max = monitored_fd_set[i];

        }
    }

    return max;
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
    //int max_fd = -1;

    int salida_fd = -1;
    char salida_buffer[OUT_BUFFSIZE];
    int pid;

    //Guardo fd en el array para el select
    monitored_fd_set[0] = consola_socket;
    monitored_fd_set[1] = mm_socket;

    fd_set readfds;

    //******Variables socket******//
    int r;

    while(TRUE) {

        //Actualizo los fd a controlar por el select.
        refresh_fd_set(&readfds);

        //Realizo un select el cual nos permite saber si usuario desea realizar una accion o MM nos envia resultados.
        select(get_max_fd() + 1, &readfds, NULL, NULL, NULL);

        if(FD_ISSET(consola_socket, &readfds)) {

        //    if( tarea_consola(consola_socket,mm_socket ,buffer, mensaje)==EXIT_FAILURE){

        //        return EXIT_FAILURE;
        //    }
            //*********RECIBE CONSOLA********//

            r = recv(consola_socket, buffer, ENTRADA_BUFFSIZE, 0);

            if (r == ERROR_CONNECTION){
                close(consola_socket);
                close(mm_socket);
                MYERR(EXIT_FAILURE, "[Rp] Error en el recv \n");
            }
            else if (r == END_OF_CONNECTION){
                close(consola_socket);
                close(mm_socket);
                MYERR(EXIT_FAILURE, "[Rp] Conexion finalizada con Consola\n");
            }

            printf("[Rp] Recibe de consola: %s \n", buffer);

            //CONVIERTO A ESTRUCTURA INTERNA DEL SERVIDOR

            conv_to_struct(&mensaje, buffer);

            //Diferenciar si se quiere enganchar a la salida de otro proceso

            if(mensaje.op == LEER_SALIDA) {

                //En mensaje.data esta el pid en formato string
                char pipe_addr[100];          //Direccion para guardar el address de la pipe
                strcpy(pipe_addr, PIPE_ADDR); //Address = /tmp/pipe_
                strcat(pipe_addr, mensaje.data); //Address = /tmp/pipe_2124

                salida_fd = open(pipe_addr, O_RDONLY);

                printf("Fd de la pipe: %d\n", salida_fd);

                sscanf(mensaje.data, "%d", &pid);

                
                if(salida_fd == FALLO) {
                    sprintf(salida_buffer, "[%d] %s", pid, "No se encontro el proceso\n");
                } else{
                    sprintf(salida_buffer, "[%d] %s", pid, "Exito al engancharse a la salida\n");    
                }

                sprintf(respuesta, "%d-%s", SINCRONICO, salida_buffer);

                send(consola_socket, respuesta, sizeof(respuesta) + 1, MSG_NOSIGNAL);

                monitored_fd_set[2] = salida_fd;

            } else{

                //*********MANDA A MM***********//

                if (send(mm_socket, &mensaje, sizeof(mensaje), MSG_NOSIGNAL) <= 0){
                    close(mm_socket);
                    MYERR(EXIT_FAILURE, "[Rp] Error en el send \n");
                }
            }
        } else if(FD_ISSET(mm_socket, &readfds)) {

            //*********RECIBE DE MM***********//
            //Rebice mensaje y lo formatea.

            r = recv(mm_socket, &mensaje, sizeof(mensaje), 0);

            if (r == ERROR_CONNECTION){
                close(mm_socket);
                MYERR(EXIT_FAILURE, "[Rp] Error en el recv \n");
            }
            else if (r == END_OF_CONNECTION){
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

        } else if( FD_ISSET(salida_fd, &readfds) ){

                r = read(salida_fd, salida_buffer, OUT_BUFFSIZE);

                if(r == ERROR_CONNECTION) {
                    perror("Error en la conexion con el listener");
                    salida_fd = -1;
                } else if(r == END_OF_CONNECTION) {
                    perror("Se termino de leer el proceso");
                    salida_fd = -1;
                } else{
                    sprintf(respuesta, "%d-%s", ASINCRONICO, salida_buffer);
                    printf("Respuesta mandada: %s\n", respuesta);
                    send(consola_socket, respuesta , sizeof(respuesta) + 1, MSG_NOSIGNAL);
                }

                

        }

   }

    return 0;
}
