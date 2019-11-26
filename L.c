
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



//----------------------INTERRUPCIONES-------------------------//
//-------------Manjeo de la interrupcion de la muerte del hijo--------------//
void sigTermHandler(int signum, siginfo_t *info, void *ucontext ) {

   close(pipe_addr);
   exit(EXIT_SUCCESS);

}

//--------------------Set del manejador de muerte del hijo-----------------//
void sigTermSet() {
    struct sigaction action, oldaction;

    action.sa_sigaction = sigTermHandler; //Funcion a llamar
    sigemptyset(&action.sa_mask);
    sigfillset(&action.sa_mask); //Bloqueo todas la seniales
    action.sa_flags = SA_NOCLDSTOP | SA_SIGINFO;
    action.sa_restorer = NULL;

    sigaction(SIGCHLD, &action, &oldaction);
}

char pipe_addr[100];

int main(int argc, char const *argv[])
{
    sigTermSet();
    

    char buffer[ENTRADA_BUFFSIZE];

    strcpy(pipe_addr, argv[1]);

    int pipe_fd = open(pipe_addr, O_RDONLY);

    if(pipe_fd == FALLO) {
        MYERR(EXIT_FAILURE, "L no pudo acceder a la pipe");
    }


    while (TRUE)
    {
        sleep(1);
    }
    
    return 0;
}
