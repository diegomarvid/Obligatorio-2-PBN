#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>
#include <error.h>
#include <errno.h>
#include "DynList.c"
#include "constantes.h"
#include "R.h"
#include "sock.c"


DynList *lista_Rp;



//----------------------INTERRUPCIONES-------------------------//
//-------------Manjeo de la interrupcion de la muerte del hijo--------------//
void sigChildHandler(int signum, siginfo_t *info, void *ucontext ) {

    int status;
    pid_t pid;

    //Espera al hijo muerto.
    pid = waitpid(-1, &status, 0);
    //Elimina al Rp asociado de su lista.
    eliminar_nodo(lista_Rp, pid);

    printf("[%d] Proceso Terminado con status %d \n", pid, status);

}

//--------------------Set del manejador de muerte del hijo-----------------//
void sigChildSet() {
    struct sigaction action, oldaction;

    action.sa_sigaction = sigChildHandler; //Funcion a llamar
    sigemptyset(&action.sa_mask);
    sigfillset(&action.sa_mask); //Bloqueo todas la seniales
    action.sa_flags = SA_NOCLDSTOP | SA_SIGINFO;
    action.sa_restorer = NULL;

    sigaction(SIGCHLD, &action, &oldaction);
}


//-------------------------CERRAR PROCESOS RP---------------------------//
//Dado el pid de un proceso cierra de forma correcta al mismo.
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

//-------------------------CREAR RP----------------------------//
pid_t crear_Rp(int sockfd, int socket) {
    
    //Hace un fork para crear Rp
    pid_t pid = fork();

    //Si falla el fork se envia mensaje de error al conectarse.
    if(pid < 0) {
        printf("Error al hacer fork \n");




        close(sockfd);

    //Si estoy en el padre agrego el nuevo Rp a la lista.
    } else if(pid > 0){
        printf("Creacion de R' exitosa, PID: %d \n", pid);
        agregar_nodo(lista_Rp, pid, -1);
        close(sockfd);
        
    //Si estoy en el hijo
    } else {

        //Convierto el fd a texto para pasarlo como argumento.
        char sock_RpStr[10];
        sprintf(sock_RpStr, "%d", sockfd);

        close(socket);
        //Ejecuto el codigo de Rp
        execlp("./Rp", "Rp" , sock_RpStr, NULL);
        printf("Error \n");
    }

    //Retorna el pid del nuevo hijo.
    return pid;

}


//--------------CODIGO DE R----------------//
int main(int argc, char const *argv[])
{

    sigChildSet();

    //Inicializo la lista de los Rp.
    lista_Rp = dynList_crear();

    //Creo un socket para conectar a las consolas con el sistema.
    int socket = sock_listen_in(PORT); 

    while(TRUE){

        
        int sock_Rp = sock_open_in(socket);

        if (sock_Rp == ERROR_CONNECTION){

            MYERR(EXIT_FAILURE, "Error, no se pudo aceptar conexion \n");

        } else {
            
            pid_t rp_pid = crear_Rp(sock_Rp,socket);

        }

    }



    // //CREAR R PRIMA
    // for(int i = 0; i < 3; i++) {
    //     pid_t pid = crear_Rp();
    //     printf("[%d] Proceso Pausado \n", pid);
    //     sleep(1);
    // }

    // printf("\n");

    // //TIEMPO MUERTO
    // sleep(3);

    // //RENAUDAR PROCESOS
    // Nodo *actual = lista_Rp->head;
    
    // while(actual != NULL){   
    //     kill(actual->data, SIGCONT);
    //     printf("[%d] Proceso Renaudado \n", actual->data);
    //     actual = actual->next;
    // }

    // //TIEMPO MUERTO
    // // sleep(3);

    // // //CERRAR PROCESOS
    // // actual = lista_Rp->head;

    // // while(actual != NULL){       
    // //     cerrar_proceso(actual->data);
    // //     sleep(1);
    // //     actual = actual->next;
    // // }

 

    // //printf("Ret: %d \n", ret);

    // //R QUEDA ESPERANDO ASI MANEJA LA MUERTE DE SUS HIJOS
    //  while(1) {
         
    //  } 

    return 0;
}

