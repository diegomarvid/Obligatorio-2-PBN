#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <error.h>
#include <sys/socket.h>
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

//-------------Manjeo de la interrupcion de Terminacion--------------//
void sigTermHandler(int signum, siginfo_t *info, void *ucontext ) {

    sistema_cerrado = TRUE;
    close(connection_socket);
    connection_socket = -1;
    printf("[R] Me estoy auto-eliminando...\n");

}

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

    if(estado == 0) {
    
        printf("[R] Estas demorando mucho... \n");
        printf("[R]Envio signal de kill a %d \n", pid);

        if(kill(pid, SIGKILL) == -1) {
            perror("Error en SIGTERM \n");
        }
        
    } else if(estado == -1) {
        //Si SIGTERM anduvo da este error, deberia manejarlo distinto
        printf("[R] Ya se elimino Rp (%d)\n", pid);
    } else {
        printf("[R] Ya se elimino el proceso (%d) \n", pid);
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

void cerrar_lista_Rp(void) {

    printf("[R] Cerrando Rp...\n");

    Nodo *actual = lista_Rp->head;
    
    while(actual != NULL){   
        cerrar_proceso(actual->data, 2);
        actual = actual->next;
    }


}


//--------------CODIGO DE R----------------//
int main(int argc, char const *argv[])
{

    sigChildSet();
    sigTermSet();

    fd_set readfds;

    //Inicializo la lista de los Rp.
    lista_Rp = dynList_crear();

    //Creo un socket para conectar a las consolas con el sistema.
    connection_socket = sock_listen_in(PORT); 

    while(!sistema_cerrado){

        //Refresh fdset
        FD_ZERO(&readfds);
        FD_SET(connection_socket, &readfds);

        select(connection_socket + 1, &readfds, NULL, NULL, NULL);

        if(FD_ISSET(connection_socket, &readfds)) {

            //----------Acepto conexion y derivo a Rp--------------//

            //printf("Esperando conexion..\n");

            int sock_Rp = sock_open_in(connection_socket);

            if (sock_Rp == ERROR_CONNECTION){
                MYERR(EXIT_FAILURE, "Error, no se pudo aceptar conexion \n");
            }
            else{        
                if (crear_Rp(sock_Rp, connection_socket) == FALLO){          
                    MYERR(EXIT_FAILURE, "Error, R no pudo crear Rp \n");
                }
            }
        }

       // printf("No veo nueva conexion \n"); 
        

    }

    cerrar_lista_Rp();

    close(connection_socket);



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

