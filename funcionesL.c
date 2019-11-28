#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <error.h>
#include <errno.h>
#include <fcntl.h> // para el mkfifo
#include <sys/stat.h> // para el mkfifo
#include <sys/types.h>// para el mkfifo
#include "constantes.h"
#include "sock.c"





int monitored_fd_set[MAX_CLIENTS];

/*Remove all the FDs, if any, from the the array*/
void intitiaze_monitor_fd_set(void){

    int i;

    for(i = 0 ; i < MAX_CLIENTS ; i++) {
        monitored_fd_set[i] = -1;
    }
}


/*Add a new FD to the monitored_fd_set array*/
void add_to_monitored_fd_set(int skt_fd){

    int i = 0;

    while (i < MAX_CLIENTS && monitored_fd_set[i] != -1){
        i++;
    }

    if(monitored_fd_set[i] == -1){
        monitored_fd_set[i] = skt_fd;
    }
}


/*Remove the FD from monitored_fd_set array*/
void remove_from_monitored_fd_set(int skt_fd){

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

int get_max_fd(void){

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
void refresh_fd_set(fd_set *fd_set_ptr){

    FD_ZERO(fd_set_ptr);

    int i;

    for(i = 0; i < MAX_CLIENTS; i++){

        if(monitored_fd_set[i] != -1){

            FD_SET(monitored_fd_set[i], fd_set_ptr);

        }
    }
}

void cerrar_sockets(void){
        int i;

        for(i = 0; i < MAX_CLIENTS; i++){
            if(monitored_fd_set[i] != -1){
                close(monitored_fd_set[i]);
            }
        } 
}

//---------------------------Handler de Terminacion-------------------------------//

void sigTermHandler(int signum, siginfo_t *info, void *ucontext) {

   cerrar_sockets();
   
   exit(EXIT_SUCCESS); 

}

//--------------------------------------------------------------------------------//

//--------------------------Set del manejador de Terminacion----------------------//
void sigTermSet(void) {
    struct sigaction action, oldaction;

    action.sa_sigaction = sigTermHandler; //Funcion a llamar
    sigemptyset(&action.sa_mask);
    sigfillset(&action.sa_mask); //Bloqueo todas la seniales
    action.sa_flags =  SA_SIGINFO;
    action.sa_restorer = NULL;

    sigaction(SIGTERM, &action, &oldaction);
}

//--------------------------------------------------------------------------------//