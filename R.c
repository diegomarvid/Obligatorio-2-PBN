#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include "DynList.c"
#include "R.h"



pid_t crear_Rp(void) {
    
    pid_t pid = fork();

    if(pid < 0) {
        printf("Error al hacer fork \n");
    } else if(pid > 0){
        printf("Estoy en el padre, mi pid es: %d, y el de mi hijo es: %d \n", getpid(), pid);
        wait(NULL);
        printf("Murio mi hijo :( \n");
    } else {
        printf("Estoy en el hijo, mi pid es: %d, y el de mi padre es: %d \n", getpid(), getppid());
        execlp("./Rp", "Rp" ,NULL);
        printf("Error \n");
    }

    

    return pid;

}




int main(int argc, char const *argv[])
{

    crear_Rp();

    return 0;
}

