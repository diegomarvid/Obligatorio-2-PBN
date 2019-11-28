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
#include <semaphore.h>
#include <sys/types.h>// para el mkfifo
#include "constantes.h"
#include "PM.h"
#include "shm.c"
#include "sock.c"

volatile Proceso *lista_proceso;
volatile int sistema_cerrado = FALSE;
sem_t *sem;


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

        sem_wait(sem);
        p = lista_proceso[i];
        sem_post(sem);

        if(p.estado != TERMINADO) {
            if(p.pid == pid) {

                sem_wait(sem);
                lista_proceso[i].estado = estado;
                sem_post(sem);

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

void crear_listener(char *pid_str, int *L_pid) {

    *L_pid = fork();

    if(*L_pid == 0) {
        execlp("./L", "L", pid_str , NULL);
        exit(EXIT_FAILURE);     
    } 

}



pid_t crear_proceso(char cmd[], int *L_pid) {


    char *comando[20];
    //Auxiliar porque la funcion split modifica
    //el array que se le pasa por parametro
    char str_aux[CMD_SIZE];

    strcpy(str_aux, cmd);
    str_split(comando, str_aux, " ");

    //Creo proceso
    pid_t pid = fork();

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

        crear_listener(pid_str, L_pid);

        if(*L_pid == FALLO) {
            return FALLO;
        }

        return pid;

        
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
    } else{
        return FALLO;
    }


}


void ejecutar_procesos(int mm_socket) {
    Proceso p;
    int i;
    pid_t pid;
    pid_t L_pid;
    Mensaje mensaje = {-1, -1, "", PM};

    while(!sistema_cerrado) {

        for(i = 0; i < PROCESS_MAX; i++) {

            sem_wait(sem);
            p = lista_proceso[i];
            sem_post(sem);

            //CREAR PROCESO
            //Se encarga de hacer fork y despues evaluar
            //y enviar un send a MM con el status de creacion

            if(p.estado == CREAR) {

                pid = crear_proceso(p.cmd, &L_pid);

                if(pid == FALLO) {

                    sem_wait(sem);
                    lista_proceso[i].pid = INVALIDO;
                    lista_proceso[i].estado = TERMINADO;
                    sem_post(sem);

                    printf("Error en el fork \n");
                    sprintf(mensaje.data, "%d-%d", pid, FALLO);

                } else {

                    sem_wait(sem);
                    lista_proceso[i].pid = pid;
                    lista_proceso[i].estado = EJECUTANDO;
                    lista_proceso[i].LID = L_pid;
                    sem_post(sem);

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

            if(p.estado == ELIMINAR) {
                kill(p.pid, SIGKILL);

                // if(kill(p.LID, SIGTERM)==-1){
                //         perror("Error en eliminar L\n");
                // }

                sem_wait(sem);
                lista_proceso[i].estado = TERMINADO;
                sem_post(sem);               
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

                // if(kill(p.LID, SIGTERM)==-1){
                //         perror("Error en eliminar L\n");
                // }

                sem_wait(sem);
                lista_proceso[i].estado = TERMINADO;
                sem_post(sem);
            }

        }
    }
}

//-------------------------CERRAR PROCESOS---------------------------//
//Dado el pid de un proceso cierra de forma correcta al mismo.
void cerrar_proceso(pid_t pid, int tiempo) {

    int status;
    int estado = 0;

    printf("[PM] Envio signal de terminate a %d \n", pid);
    
    if(kill(pid, SIGTERM) == -1) {
        perror("Error en SIGTERM \n");
    };

    sleep(tiempo);

    estado = waitpid(pid, &status, WNOHANG);

    printf("[PM] Estado de waitpid WNOHANG: %d \n", estado);

    if(estado == 0) {
        printf("[PM] Estas demorando mucho... \n");
        printf("[PM] Envio signal de kill a %d \n", pid);

        if(kill(pid, SIGKILL) == -1) {
            perror("Error en SIGKILL \n");
        }
    }else if(estado == -1) {
        //Si SIGTERM anduvo da este error, deberia manejarlo distinto
        //perror("Error en waitpid WNOHANG \n");
        printf("[PM] Ya se elimino el proceso (%d)\n", pid);
    } else {
        printf("[PM] Ya se elimino el proceso (%d)\n", pid);
    }


}


void eliminar_procesos(void) {

    Proceso p;
    int i;

    for(i = 0; i < PROCESS_MAX; i++) {

        sem_wait(sem);
        p = lista_proceso[i];
        sem_post(sem);

        if(p.estado != TERMINADO) {
            cerrar_proceso(p.pid, 1);
        }
    }

    
}
