#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include "constantes.h"
#include "Rp.h"

volatile sig_atomic_t done = FALSE;
volatile sig_atomic_t porcentaje = 0;

void sigTermHandler(int signum, siginfo_t *info, void *ucontext ) {
    
    while(porcentaje <= 100) {
        printf("[%d] Finalizando %d %%\n", getpid(), porcentaje);
        usleep(500 * 1000);
        porcentaje++;
    }
    //done = TRUE;    
  
}

void sigTermSet() {
    struct sigaction action, oldaction;

    action.sa_sigaction = sigTermHandler; //Funcion a llamar
    sigemptyset(&action.sa_mask);
    sigfillset(&action.sa_mask); //Bloqueo todas la seniales
    action.sa_flags = SA_SIGINFO;
    action.sa_restorer = NULL;

    sigaction(SIGTERM, &action, &oldaction);
}



int main(int argc, char const *argv[])
{
    sigTermSet();

    int i;

    while(i < 10 && done == FALSE) {
        printf("[%d] %d \n", getpid() , i);
        sleep(1);
        i++;
    }
        
    

    while(!done) {
        
    }

    return 0;
}
