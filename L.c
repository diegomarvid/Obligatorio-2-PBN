
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <error.h>
#include <errno.h>
#include <fcntl.h> // para el mkfifo
#include <sys/stat.h> // para el mkfifo
#include <sys/types.h>// para el mkfifo
#include "constantes.h"
#include "sock.c"

int monitored_fd_set[MAX_CLIENTS];

/*Remove all the FDs, if any, from the the array*/
static void
intitiaze_monitor_fd_set(){

    int i;

    for(i = 0 ; i < MAX_CLIENTS ; i++) {
        monitored_fd_set[i] = -1;
    }
}


/*Add a new FD to the monitored_fd_set array*/
static void
add_to_monitored_fd_set(int skt_fd){

    int i = 0;

    while (i < MAX_CLIENTS && monitored_fd_set[i] != -1){
        i++;
    }

    if(monitored_fd_set[i] == -1){
        monitored_fd_set[i] = skt_fd;
    }
}


/*Remove the FD from monitored_fd_set array*/
static void
remove_from_monitored_fd_set(int skt_fd){

    int i = 0;

    while (i < MAX_CLIENTS && monitored_fd_set[i] != skt_fd){
        i++;
    }

    if (monitored_fd_set[i] == skt_fd){
        monitored_fd_set[i] = -1;
    }
}


/*Get the numerical max value among all FDs which server
 * is monitoring*/

static int
get_max_fd(){

    int i;
    int max = -1;

    for(i = 0 ; i < MAX_CLIENTS; i++ ){

        if(monitored_fd_set[i] > max){

            max = monitored_fd_set[i];

        }
    }

    return max;
}


/* Clone all the FDs in monitored_fd_set array into
 * fd_set Data structure*/
static void
refresh_fd_set(fd_set *fd_set_ptr){

    FD_ZERO(fd_set_ptr);

    int i;

    for(i = 0; i < MAX_CLIENTS; i++){

        if(monitored_fd_set[i] != -1){

            FD_SET(monitored_fd_set[i], fd_set_ptr);

        }
    }
}

void cerrar_sockets(){
        int i;

        for(i = 0; i < MAX_CLIENTS; i++){
            if(monitored_fd_set[i] != -1){
                close(monitored_fd_set[i]);
            }
        } 
}


int main(int argc, char const *argv[])
{
    
    char sock_addr [100];

    char pipe_addr[100];

    char buffer[RESPUESTA_BUFFSIZE - 2];

    int connection_socket;
    int data_socket;
    int socket_actual;
    //int rp_connections[MAX_CLIENTS];

    int proceso_vivo = TRUE;

    int i;

    fd_set readfds;

    //strcpy(pid_addr, argv[1]);

    strcpy(sock_addr, L_ADDR); //sock_addr = /tmp/listener_
    strcpy(pipe_addr, PIPE_ADDR); //pipe_addr = /tmp/pipe_

    strcat(sock_addr,argv[1]); //sock_addr = /tmp/listener_2048
    strcat(pipe_addr,argv[1]); //pipe_addr = /tmp/pipe_2048

    intitiaze_monitor_fd_set();


    /*****CONEXION PIPE*****/

    int pipe_fd = open(pipe_addr, O_RDONLY);
    
    if(pipe_fd == FALLO) {
        MYERR(EXIT_FAILURE, "L no pudo acceder a la pipe");
    }

    /*****CONEXION SOCKET*****/
    unlink(sock_addr);

    //Master socket fd para aceptar conexiones
    connection_socket = sock_listen_un(sock_addr);

    if( connection_socket < 0 ){

        MYERR(EXIT_FAILURE, "Error, no se pudo crear server. \n");

    }

    add_to_monitored_fd_set(connection_socket);
    add_to_monitored_fd_set(pipe_fd);

    while(proceso_vivo) {

        refresh_fd_set(&readfds);

        printf("Waiting on select sys call \n");

        select(get_max_fd() + 1, &readfds, NULL, NULL, NULL);

        //Manejo el accept de un Rp para escuchar
        if(FD_ISSET(connection_socket, &readfds)){


            data_socket = sock_open_un(connection_socket);

            if( data_socket == ERROR_CONNECTION ){
                MYERR(EXIT_FAILURE, "Error, no se pudo aceptar conexion. \n");
            }

            add_to_monitored_fd_set(data_socket);
            

        }else if(FD_ISSET(pipe_fd, &readfds)){

            int w;
            int r;

            memset(buffer, 0, RESPUESTA_BUFFSIZE - 2);

            r = read(pipe_fd, buffer, RESPUESTA_BUFFSIZE - 2);

            if (r == ERROR_CONNECTION){
                perror("Error en la conexion con el listener");
                //close(socket_actual)
                cerrar_sockets();
                pipe_fd = -1;
                proceso_vivo = FALSE;
                //Eliminar L()
            }
            else if (r == END_OF_CONNECTION){       
                perror("Se termino de leer el proceso");
                proceso_vivo = FALSE;
                //close(socket_actual)
                cerrar_sockets();
                pipe_fd = -1;

                //Eliminar L()
            }
            else{

                for(i = 0; i < MAX_CLIENTS; i++){

                    socket_actual = monitored_fd_set[i];

                    if(socket_actual != connection_socket && socket_actual != pipe_fd && socket_actual != -1) {
                        printf("Respuesta mandada: %s\n", buffer);
                        w = send(socket_actual, buffer, r, MSG_NOSIGNAL);
                        printf("Cantidad bytes recibidos: %d \nCantidad de bytes enviados: %d\n", r, w);

                        if(w < 0) {
                            perror("Error en escritura de L");
                            remove_from_monitored_fd_set(socket_actual);
                            close(socket_actual);
                        }
                    }

                } 
            } 

        }
    }
  
    return 0;
}
