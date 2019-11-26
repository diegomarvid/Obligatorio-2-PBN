#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <error.h>
#include <errno.h>
#include <fcntl.h> // para el mkfifo
#include <sys/stat.h> // para el mkfifo
#include <sys/types.h>// para el mkfifo
#include "constantes.h"
#include "PM.h"
#include "shm.c"
#include "sock.c"


volatile Proceso *lista_proceso;

/*  Funcion para separar un string en un array de string por un delimitador  */

void str_split(char *build_string[], char string[], char *delim) {
    char *token = strtok(string, delim);
    int length = 0;

    while (token != NULL) {
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

int crear_name_pipe(char *pipe_addr) {

    unlink(pipe_addr);

    if(mkfifo(pipe_addr, 0666) == FALLO){
        return FALLO;
    }

    return EXITO;

}

pid_t crear_listener(char *pipe_addr) {

    pid_t pid_L = fork();

    if(pid_L < 0) {
        return FALLO;

    } else if(pid_L == 0) {
        execlp("./L", "L", pipe_addr , NULL);
        exit(EXIT_FAILURE);     
    } else{
        return pid_L;
    }

}



pid_t crear_proceso(char cmd[]) {


    char *comando[20];
    //Auxiliar porque la funcion split modifica
    //el array que se le pasa por parametro
    char str_aux[CMD_SIZE];

    strcpy(str_aux, cmd);
    str_split(comando, str_aux, " ");

    //Creo proceso
    pid_t pid = fork();

    if(pid < 0) {
        return FALLO;
    }

    char pipe_addr[100];          //Direccion para guardar el address de la pipe
    strcpy(pipe_addr, PIPE_ADDR); //Address = /tmp/pipe_
    char pid_str[20];

    if(pid > 0) {

        printf("\n\n[%d] Proceso creado \n\n", pid);

        
        sprintf(pid_str, "%d", pid); // Paso el pid a str para concatenarlo
        strcat(pipe_addr, pid_str);  //Address = /tmp/pipe_2180

        if (crear_name_pipe(pipe_addr) == FALLO)
        {
            return FALLO;
        }

        if(crear_listener(pipe_addr) == FALLO) {
            return FALLO;
        }

        
    } else if(pid == 0) {

        sprintf(pid_str, "%d", getpid()); // Paso el pid a str para concatenarlo
        strcat(pipe_addr, pid_str); //Address = /tmp/pipe_2180

        
        printf("Me tranque \n");

        //Se abre como RDWR para que no tranque cuando no hay lectores
        int pipe_fd = open(pipe_addr, O_WRONLY);

        printf("Me destranque \n");

        if(pipe_fd == FALLO) {
            MYERR(EXIT_FAILURE, "Error al abrir pipe para escritura");
        }

        dup2(pipe_fd, STDOUT_FILENO);
    
        execvp(comando[0], &comando[0]);

        exit(EXIT_FAILURE);
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

            //CREAR PROCESO
            //Se encarga de hacer fork y despues evaluar
            //y enviar un send a MM con el status de creacion

            if(p.estado == CREAR) {

                pid = crear_proceso(p.cmd);

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

                if(send(mm_socket, &mensaje, sizeof(mensaje), MSG_NOSIGNAL) <= 0) {
                    MYERR(EXIT_FAILURE, "Error en el send");
                }
             }

            if(p.estado == EJECUTANDO) {
                kill(p.pid, SIGCONT);
                sleep(5);
                kill(p.pid, SIGSTOP);
            }

            //Cuando un proceso muere con status de error
            //se cambia su estado a invalido, es por esto
            //que si se encuentra que un proceso murio
            //se envia la falla y se cambia a terminado
            //para convertirlo en espacio libre.

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

    lista_proceso = obtener_shm(OFFSET);

    ejecutar_procesos(mm_socket);


    return 0;
}
