#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include "sock.c"
#include "constantes.h"
#include "MM.h"
#include <error.h>
#include <errno.h>


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
    // for( i = 0 ; i < MAX_CLIENT_SUPPORTED ; i++){

    //     if(monitored_fd_set[i] != -1)
    //         continue;
    //     monitored_fd_set[i] = skt_fd;
    //     break;
    // }

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

    // monitored_fd_set[i] = skt_fd;
    // for(; i < MAX_CLIENT_SUPPORTED; i++){

    //     if(monitored_fd_set[i] != skt_fd)
    //         continue;

    //     monitored_fd_set[i] = -1;
    //     break;
    // }
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



int main(int argc, char const *argv[]){

    int sock_Rp;
    int connection_socket;
    fd_set readfds;
    int i;
    int socket_actual = -1;
    char buffer[BUFFSIZE] = "Soy mm chota grande";
    Mensaje mensaje;

    intitiaze_monitor_fd_set();


    unlink(SOCKET_NAME);

    connection_socket = sock_listen_un();


    if( connection_socket < 0 ){

        MYERR(EXIT_FAILURE, "Error, no se pudo crear server. \n");

    }

    add_to_monitored_fd_set(connection_socket);
    

    while (TRUE){

        refresh_fd_set(&readfds);

        printf("Waiting on select sys call \n");

        select(get_max_fd() + 1, &readfds, NULL, NULL, NULL);

        if(FD_ISSET(connection_socket, &readfds)){

    
            sock_Rp = sock_open_un(connection_socket);

            if( sock_Rp < 0 ){

                MYERR(EXIT_FAILURE, "Error, no se pudo aceptar conexion. \n");

            }

            add_to_monitored_fd_set(sock_Rp);


        }else{
            
            socket_actual = -1;

            for(i = 0; i < MAX_CLIENTS; i++){

                if(FD_ISSET(monitored_fd_set[i], &readfds)){

                    socket_actual = monitored_fd_set[i];

                    //Recibo peticion de Rp

                    int read = recv(socket_actual, &mensaje, sizeof(mensaje), 0);

                    

                    printf("[MM] recibe de Rp: \n");
                    printf("Op: %d \n", mensaje.op);
                    printf("Data: %s \n", mensaje.data);

                    //Procesamiento

                    //Envio respuesta a Rp

                    send(socket_actual, buffer, strlen(buffer) + 1, MSG_NOSIGNAL);

                    printf("[MM]manda: %s\n", buffer);
            
                    remove_from_monitored_fd_set(socket_actual);

                }

            

            }

            








        }

    }

    // printf("Me cree de forma correcta:D\n");


    // int sock_Rp;

    // sock_Rp = sock_open_un(connection_socket);

    // if( sock_Rp < 0 ){

    //     MYERR(EXIT_FAILURE, "Error, no se pudo aceptar conexion. \n");

    // }

    // printf("Se me conecto un Rp y su socket es %d \n",sock_Rp);


       
    //ejemplo(print);
    
    return 0;
}
