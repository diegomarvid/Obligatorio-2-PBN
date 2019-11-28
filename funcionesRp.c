#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <error.h>
#include <errno.h>
#include <fcntl.h> // para el mkfifo
#include <sys/stat.h> // para el mkfifo
#include <sys/types.h>// para el mkfifo
#include <signal.h>
#include "sock.c"
#include "constantes.h"
#include "Rp.h"


int monitored_fd_set[3] = {-1, -1, -1};
int mm_socket;
int consola_socket;
int salida_fd = -1;


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

int get_max_fd(){

    int i;
    int max = -1;

    for(i = 0 ; i < 3; i++ ){

        if(monitored_fd_set[i] > max){

            max = monitored_fd_set[i];

        }
    }

    return max;
}



//-----------------------Manejo de Interrupciones--------------------//

//-------------Manjeo de la interrupcion de Terminacion---------------//
void sigTermHandler(int signum, siginfo_t *info, void *ucontext) {

    char buffer[RESPUESTA_BUFFSIZE] = "Cerrando sistema...\n";

    if(send(consola_socket, buffer, strlen(buffer) + 1, MSG_NOSIGNAL) <= 0) {
        printf("[Rp] Error enviando mensaje de cierre a consola\n");
    }

    printf("[Rp] Eliminando consola\n");

    close(mm_socket);
    close(consola_socket);
    close(salida_fd);

    exit(EXIT_SUCCESS);

    

}

//--------------------Set del manejador de Terminacion-----------------//
void sigTermSet(void) {
    struct sigaction action, oldaction;

    action.sa_sigaction = sigTermHandler; //Funcion a llamar
    sigemptyset(&action.sa_mask);
    sigfillset(&action.sa_mask); //Bloqueo todas la seniales
    action.sa_flags =  SA_SIGINFO;
    action.sa_restorer = NULL;

    sigaction(SIGTERM, &action, &oldaction);
}
