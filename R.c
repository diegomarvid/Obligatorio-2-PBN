#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <signal.h>
#include "DynList.c"
#include "R.h"


DynList *lista_Rp;

void cerrar_proceso(pid_t pid) {

    int status;
    int estado;

    printf("Envio signal de terminate a %d \n", pid);
    sleep(1);

    if(kill(pid, SIGTERM) == -1) {
        perror("Error en SIGTERM \n");
    };

    sleep(10);

    estado = waitpid(pid, &status, WNOHANG);

    if(estado == 0) {
    
        printf("Estas demorando mucho... \n");
        printf("Envio signal de kill a %d \n", pid);

        if(kill(pid, SIGKILL) == -1) {
            perror("Error en SIGTERM \n");
        }
        
    } else if(estado == -1) {
        //Si SIGTERM anduvo da este error, deberia manejarlo distinto
        perror("Error en waitpid WNOHANG \n");
    } else {
        printf("Se cerro con exito \n");
    }



}

void sigChildHandler(int signum, siginfo_t *info, void *ucontext ) {

    int status;
    pid_t pid;

    pid = waitpid(-1, &status, 0);

    eliminar_nodo(lista_Rp, pid);

    printf("[%d] Proceso Terminado \n", pid);    
  

}

void sigChildSet() {
    struct sigaction action, oldaction;

    action.sa_sigaction = sigChildHandler; //Funcion a llamar
    sigemptyset(&action.sa_mask);
    sigfillset(&action.sa_mask); //Bloqueo todas la seniales
    action.sa_flags = SA_NOCLDSTOP | SA_SIGINFO;
    action.sa_restorer = NULL;

    sigaction(SIGCHLD, &action, &oldaction);
}


pid_t crear_Rp(void) {
    
    pid_t pid = fork();

    if(pid < 0) {
        printf("Error al hacer fork \n");
    } else if(pid > 0){
     
        kill(pid, SIGSTOP); //Te pause perra
        printf("Creacion de R' exitosa, PID: %d \n", pid);
        agregar_nodo(lista_Rp, pid);
        

    } else {
        execlp("./Rp", "Rp" ,NULL);
        printf("Error \n");
    }

    return pid;

}



int main(int argc, char const *argv[])
{

    sigChildSet();

    lista_Rp = dynList_crear();

    //CREAR R PRIMA
    for(int i = 0; i < 3; i++) {
        pid_t pid = crear_Rp();
        printf("[%d] Proceso Pausado \n", pid);
        sleep(1);
    }

    printf("\n");

    //TIEMPO MUERTO
    sleep(3);

    //RENAUDAR PROCESOS
    Nodo *actual = lista_Rp->head;
    
    while(actual != NULL){   
        kill(actual->data, SIGCONT);
        printf("[%d] Proceso Renaudado \n", actual->data);
        actual = actual->next;
    }

    //TIEMPO MUERTO
    sleep(3);

    //CERRAR PROCESOS
    actual = lista_Rp->head;

    while(actual != NULL){       
        cerrar_proceso(actual->data);
        sleep(1);
        actual = actual->next;
    }

    //R QUEDA ESPERANDO ASI MANAGEA LA MUERTE DE SUS HIJOS
     while(1) {
         
     } 

    return 0;
}

