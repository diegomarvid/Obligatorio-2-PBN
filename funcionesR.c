#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <error.h>
#include <sys/socket.h>
#include <signal.h>
#include <errno.h>
#include "DynList.c"
#include "constantes.h"
#include "R.h"
#include "sock.c"


DynList *lista_Rp;
volatile int sistema_cerrado = FALSE;
int connection_socket;



//----------------------INTERRUPCIONES-------------------------//



//-------------Manjeo de la interrupcion de la muerte del hijo--------------//
void sigChildHandler(int signum, siginfo_t *info, void *ucontext ) {

    int status;
    pid_t pid;

    //Espera al hijo muerto.
    pid = waitpid(-1, &status, 0);
    //Elimina al Rp asociado de su lista.
    eliminar_nodo(lista_Rp, pid);

    printf("[R] Se elimino Rp (%d)\n", pid);

}
//-------------------------------------------------------------------------//



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
//--------------------------------------------------------------//



//-------------Manjeo de la interrupcion de Terminacion--------------//
void sigTermHandler(int signum, siginfo_t *info, void *ucontext ) {

    sistema_cerrado = TRUE;

    //Cerrar y poner connection_socket a invalido asi 
    //no tranca el accept y va a eliminar Rp
    close(connection_socket);
    connection_socket = -1;
    printf("[R] Me estoy auto-eliminando...\n");

}
//--------------------------------------------------------------//



//--------------------Set del manejador de Terminacion-----------------//
void sigTermSet() {
    struct sigaction action, oldaction;

    action.sa_sigaction = sigTermHandler; //Funcion a llamar
    sigemptyset(&action.sa_mask);
    sigfillset(&action.sa_mask); //Bloqueo todas la seniales
    action.sa_flags =  SA_SIGINFO;
    action.sa_restorer = NULL;

    sigaction(SIGTERM, &action, &oldaction);
}
//--------------------------------------------------------------//



//-------------------------CERRAR PROCESOS RP---------------------------//
//Dado el pid de un proceso cierra de forma correcta al mismo.
void cerrar_proceso(pid_t pid, int tiempo) {

    int status;
    int estado;

    printf("[R] Envio signal de terminate a %d \n", pid);
    
    if(kill(pid, SIGTERM) == -1) {
        perror("Error en SIGTERM \n");
    };

    sleep(tiempo);

    estado = waitpid(pid, &status, WNOHANG);

    //Si el estado es 0 es porque el hijo no cambio de estado 
    //en el tiempo previsto
    if(estado == 0) {
    
        printf("[R] Estas demorando mucho... \n");
        printf("[R]Envio signal de kill a %d \n", pid);

        if(kill(pid, SIGKILL) == -1) {
            perror("Error en SIGTERM \n");
        }
        
    //Si no es cero o es el pid o es error significando que el hijo ya no existe
    //de cualquier manera el proceso queda eliminado.    
    } else {
        printf("[R] Ya se elimino el proceso (%d) \n", pid);
    }

}
//--------------------------------------------------------------//



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
//-------------------------------------------------------------//



//-------------------------CREAR RP----------------------------//

void cerrar_lista_Rp(void) {

    printf("[R] Cerrando Rp...\n");

    Nodo *actual = lista_Rp->head;
    Nodo *aux = actual;
    
    while(actual != NULL){   
        cerrar_proceso(actual->data, 2);
        aux = actual;
        actual = actual->next;
        free(aux);
    }

    //Despues de eliminar todos los nodos se elimina el espacio
    //de memoria de la lista
    free(lista_Rp);

}

//--------------------------------------------------------------//