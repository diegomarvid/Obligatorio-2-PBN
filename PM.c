#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <error.h>
#include <errno.h>
#include "constantes.h"
#include "PM.h"
#include "shm.c"
#include "sock.c"


volatile Proceso *lista_proceso;

//TEST
volatile int pid_creado;
volatile int proceso_creado = FALSE;
volatile int exito_creacion = TRUE;

//Funcion string
void str_split(char *build_string[], char string[], char *delim) {
    char *token = strtok(string, delim);
    int length = 0;

    while (token != NULL) {
        //printf("%s\n", token);
        build_string[length] = token;
        token = strtok(NULL, delim);

        length++;
    }
    build_string[length] = NULL;
}


//*****Funciones lista proceso******//


int cambiar_estado_proceso(pid_t pid, int estado) {
    Proceso p;
    int i;

    for(i = 0; i < PROCESS_MAX; i++) {

        p = lista_proceso[i];

        if(p.estado != TERMINADO) {
            if(p.pid == pid) {
                lista_proceso[i].estado = estado;
                return EXITO;
            }
        }
    }

    return FALLO;
}


pid_t crear_proceso(char cmd[]) {
    char *comando[20];
    //Auxiliar porque la funcion split modifica
    //el array que se le pasa por parametro
    char str_aux[CMD_SIZE];

    strcpy(str_aux, cmd);
    str_split(comando, str_aux, " ");

    pid_t pid = fork();
    if(pid < 0) {
        return FALLO;

    } else if(pid == 0){
        execvp(comando[0], &comando[0]);
        exit(EXIT_FAILURE);

    } else {
        printf("\n\n[%d] Proceso creado \n\n", pid);
    }

    return pid;
}


void ejecutar_procesos(int mm_socket) {
    Proceso p;
    int i;
    pid_t pid;
    Mensaje mensaje = {-1, -1, "", PM};

    while(TRUE) {

        for(i = 0; i < PROCESS_MAX; i++) {

            p = lista_proceso[i];

            if(p.estado == CREAR) {

                pid = crear_proceso(p.cmd);

                printf("Crear: %d de %d \n", pid, getpid());

                if(pid == FALLO) {
                    lista_proceso[i].pid = TERMINADO;
                    printf("Error en el fork \n");

                    sprintf(mensaje.data, "%d-%d", pid, FALLO);

                } else {      
                    lista_proceso[i].pid = pid;
                    lista_proceso[i].estado = EJECUTANDO;
                   
                    sprintf(mensaje.data, "%d-%d", pid, EXITO);
                             
                }

                mensaje.op = CREACION;
                mensaje.RID = p.RID;

                printf("PM mandaaaaa\n");

                if(send(mm_socket, &mensaje, sizeof(mensaje), MSG_NOSIGNAL) <= 0) {
                    MYERR(EXIT_FAILURE, "Error en el send");
                }
             }

            if(p.estado == EJECUTANDO) {
                kill(p.pid, SIGCONT);
                sleep(5);
                kill(p.pid, SIGSTOP);
            }

            if(p.estado == INVALIDO) {
                mensaje.op = CREACION;
                mensaje.RID = p.RID;

                sprintf(mensaje.data, "%d-%d", p.pid, FALLO);
                
                if(send(mm_socket, &mensaje, sizeof(mensaje), MSG_NOSIGNAL) <= 0) {
                    MYERR(EXIT_FAILURE, "Error en el send");
                }

                lista_proceso[i].estado = TERMINADO;
            }
 
        }
    }
}


//*******Funciones de signals********//

void sigChildHandler(int signum, siginfo_t *info, void *ucontext ) {

    int status;
    pid_t pid;

    pid = waitpid(-1, &status, 0);

    if(pid == FALLO) {
      printf("Fallo el waitpid \n");
    }

    printf("[Signal] status: %d \n", status);

    if(status == EXIT_SUCCESS) {
      cambiar_estado_proceso(pid, TERMINADO);
      printf("[%d] Proceso terminado \n", pid);
    } else {
      cambiar_estado_proceso(pid, INVALIDO);
      printf("[%d] Proceso invalido \n", pid);
    }




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








int main(int argc, char const *argv[])
{


    //Seteo interrupciones para Child
    sigChildSet();

    int mm_socket;
    mm_socket = sock_connect_un();

    if( mm_socket < 0 ){
        MYERR(EXIT_FAILURE, "Error, no se pudo aceptar conexion. \n");
    }

    printf("Connection established with MM \n");

    lista_proceso = obtener_shm(0);

    ejecutar_procesos(mm_socket);





    return 0;
}
