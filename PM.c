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

Proceso lista_proceso[PROCESS_MAX];


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

void iniciar_lista_proceso(){

    Proceso p;
    p.RID = INVALIDO;
    p.pid = INVALIDO;
    p.estado = TERMINADO;
    strcpy(p.cmd, "");

    int i;

    for(i = 0; i < PROCESS_MAX; i++) {
      
        lista_proceso[i] = p;

    }

}

int request_process_space() {

    Proceso p;
    int i;

    for(i = 0; i < PROCESS_MAX; i++) {
        p = lista_proceso[i];
        if(p.estado == TERMINADO){
            return i;
        }
    }

    return FALLO;

}

void print_proceso(pid_t pid) {
    Proceso p;
    int i;

    for(i = 0; i < PROCESS_MAX; i++) {

        p = lista_proceso[i];

        if(p.pid == pid){


            printf("Proceso \n");
            
            printf("RID: %d \n", p.RID);
            printf("PID: %d \n", p.pid);

            if(p.estado == INVALIDO) {
                printf("Estado: Invalido \n");
            } else if(p.estado == EJECUTANDO) {
                printf("Estado: Ejecutando \n");
            } else if(p.estado == SUSPENDIDO) {
                printf("Estado: Suspendido \n");
            } else if(p.estado == TERMINADO) {
                printf("Estado: Terminado \n");
            }

            
            printf("Cmd: %s \n", p.cmd);

            printf("\n");
        }
    }
}

void print_lista_proceso() {

    Proceso p;
    int i;

    for(i = 0; i < PROCESS_MAX; i++) {

        p = lista_proceso[i];
        
        if(p.estado != TERMINADO) {

            printf("Proceso: \n");
            printf("RID: %d \n", p.RID);
            printf("PID: %d \n", p.pid);

            if(p.estado == INVALIDO) {
                printf("Estado: Invalido \n");
            } else if(p.estado == EJECUTANDO) {
                printf("Estado: Ejecutando \n");
            } else if(p.estado == SUSPENDIDO) {
                printf("Estado: Suspendido \n");
            } else if(p.estado == TERMINADO) {
                printf("Estado: Terminado \n");
            }

            
            printf("Cmd: %s \n", p.cmd);

            printf("\n");

        }
   
    }

}

int agregar_proceso(pid_t rid, pid_t pid, int estado, char cmd[]){
    
    int indice = request_process_space();

    if(indice != FALLO) {

        Proceso p;
        p.RID = rid;
        p.pid = pid;
        p.estado = estado;
        strcpy(p.cmd, cmd);

        lista_proceso[indice] = p;

        return indice;

    } else{

        return FALLO;
    }
    


}


void ejecutar_procesos() {


    while(1) {

        Proceso p;
        int i;

        for(i = 0; i < PROCESS_MAX; i++) {

            p = lista_proceso[i];

            if(p.estado == EJECUTANDO) {
                kill(p.pid, SIGCONT);
                usleep(500 * 1000);
                kill(p.pid, SIGSTOP);
            }

        }

    }


}


void sigChildHandler(int signum, siginfo_t *info, void *ucontext ) {

    int status;
    pid_t pid;

    pid = waitpid(-1, &status, 0);

    cambiar_estado_proceso(pid, TERMINADO);

	printf("[%d] Proceso terminado \n", pid);

   
  
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

pid_t crear_proceso_pausado(char *comando[], char cmd[]) {

    pid_t pid = fork();

    if(pid < 0) {

        perror("Error al hacer fork \n");

    } else if(pid == 0){

        execvp(comando[0], &comando[0]);

        perror("Error en la ejecucion del hijo \n");
     
    } else {

        kill(pid, SIGSTOP);

        if(agregar_proceso(getpid(), pid, EJECUTANDO, cmd) == -1){
            printf("Error en espacio de memoria \n");
        }    

        printf("[%d] Proceso creado \n", pid);
        
    }

    return pid;

}

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

int clean_stdin()
{
    while (getchar()!='\n');
    return 1;
}


int main(int argc, char const *argv[])
{
    //Seteo interrupciones para Child
    sigChildSet();

    //Iniciar lista de procesos
    iniciar_lista_proceso();

    for(int i = 0; i < 3; i++) {

        //Comando a ejecutar
        char string[CMD_SIZE];

        printf("Ingrese comando a ejecutar: \n");

        if (scanf("%[^\n]", string) == -1){
            MYERR(EXIT_FAILURE, "Error en el comando \n");
        }

        clean_stdin();

        //Array para excevp
        char *comando[20];
        //Auxiliar para que split no modifique string
        char str_aux[CMD_SIZE];
        strcpy(str_aux, string);

        //Cargo los parametros del string en el array
        str_split(comando, str_aux, " ");

        //Creo proceso pausado
        pid_t pid = crear_proceso_pausado(comando, string);

        //sleep(3);

        //print_lista_proceso();

    }

	sleep(3);
    printf("\n");
    //print_lista_proceso();
    //sleep(3);
    printf("Ahora se ejecutaran los procesos: \n");
    sleep(1);

    ejecutar_procesos();

	//El padre queda esperando para manejar la muerte del hijo
    while(1){

    }

    return 0;
}