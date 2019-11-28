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


//---------------------VARIABLES GLOBALES---------------------//

volatile Proceso *lista_proceso; //Lista de shm
volatile int sistema_cerrado = FALSE; //Estado del sistema
sem_t *sem; //Semaforo


//------------------------FUNCIONES---------------------------//


//--------------------Manejo de SIGCHILD----------------------//

void sigChildHandler(int signum, siginfo_t *info, void *ucontext ) {


    //Si el sistema se cerro el handler de muerte de hijo lo
    //maneja la funcion de eliminar procesos
    if(sistema_cerrado == TRUE) {
        return;
    }

    int status;
    pid_t pid;

    pid = waitpid(-1, &status, 0);

    if(pid == FALLO) {
      printf("Fallo el waitpid \n");
    } else {

        //---------Obtener direccion de la pipe para cerrarla-----------//

        char pipe_addr[100];          //Direccion para guardar el address de la pipe
        strcpy(pipe_addr, PIPE_ADDR); //Address = /tmp/pipe_
        char pid_str[20];
        sprintf(pid_str, "%d", pid); // Paso el pid a str para concatenarlo
        strcat(pipe_addr, pid_str);  //Address = /tmp/pipe_2180

        unlink(pipe_addr);
    }


    //Se evalua si interrumpio un semaforo

    int semval;
    int interrumpi_sem = FALSE;
    sem_getvalue(sem,&semval);

    //Si llega a la interrumpcion con semaforo en 0 es porque 
    //interrumpio el semaforo, por eso se le incrementa 1
    //para poder usar la shm dentro de la signal
    if(semval == 0){
        sem_post(sem);
        interrumpi_sem = TRUE;
    }


    //Si termina mal el proceso se setea el estado a invalido
    //Para despues mandar un mensaje de error y se elimina
    if(status == EXIT_SUCCESS) {
      cambiar_estado_proceso(pid, ELIMINAR);
    } else {
      cambiar_estado_proceso(pid, INVALIDO);  
    }

    printf("[PM] - [%d] Proceso terminado\n", pid);

    sem_getvalue(sem,&semval);

    //Si interrumpio y se uso la shm el semaforo
    //debe volver a 0 para volver como estaba antes
    //Por eso se le resta 1
    if(semval == 1 && interrumpi_sem) {
        sem_wait(sem);
    }

}

//-----------------------------------------------------------//



//--------------------Seteo de SIGCHILD----------------------//

void sigChildSet(void) {
    struct sigaction action, oldaction;

    action.sa_sigaction = sigChildHandler; //Funcion a llamar
    sigemptyset(&action.sa_mask);
    sigfillset(&action.sa_mask); //Bloqueo todas la seniales
    action.sa_flags = SA_NOCLDSTOP | SA_SIGINFO;
    action.sa_restorer = NULL;

    sigaction(SIGCHLD, &action, &oldaction);
}

//--------------------Manejo de SIGTERM----------------------//

void sigTermHandler(int signum, siginfo_t *info, void *ucontext) {

    sistema_cerrado = TRUE;
    printf("[PM] Me estoy autoeliminando...\n");

}

//----------------------------------------------------------//



//--------------------Seteo de SIGTERM----------------------//

void sigTermSet(void) {
    struct sigaction action, oldaction;

    action.sa_sigaction = sigTermHandler; //Funcion a llamar
    sigemptyset(&action.sa_mask);
    sigfillset(&action.sa_mask); //Bloqueo todas la seniales
    action.sa_flags = SA_SIGINFO;
    action.sa_restorer = NULL;

    sigaction(SIGTERM, &action, &oldaction);
}

//----------------------------------------------------------//


//-----------Separar un string por un delimitador en un array--------------//

void str_split(char *build_string[], char string[], char *delim) {

    //En build string se guarda el array de strings nuevo
    //String es el string que se recorre para separarlo
    //La funcion modifica string por eso se recomienda
    //usar una copia para pasarle en string.

    char *token = strtok(string, delim);
    int length = 0;

    while (token != NULL) {
        build_string[length] = token;
        token = strtok(NULL, delim);
        length++;
    }

    build_string[length] = NULL;
}

//----------------------------------------------------------//



//--------------Cambiar estado del proceso------------------//

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

//----------------------------------------------------------//



//-------------------Crear name pipe------------------------//

int crear_name_pipe(char *pipe_addr) {

    unlink(pipe_addr);

    if(mkfifo(pipe_addr, 0666) == FALLO){
        return FALLO;
    }

    return EXITO;

}

//----------------------------------------------------------//



//---------------------Crear listener-----------------------//


void crear_listener(char *pid_str, int *L_pid) {

    *L_pid = fork();

    if(*L_pid == 0) {
        execlp("./L", "L", pid_str , NULL);
        exit(EXIT_FAILURE);     
    } 

}

//----------------------------------------------------------//




//---------------------Crear proceso------------------------//

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

        sprintf(pid_str, "%d", pid); // Paso el pid a str para concatenarlo
        strcat(pipe_addr, pid_str);  //Address = /tmp/pipe_2180

        if (crear_name_pipe(pipe_addr) == FALLO)
        {
            return FALLO;
        }

        //Le paso el pid en str para que sepa a donde conectarse
        //Y guarda su pid en la variable L_pid
        crear_listener(pid_str, L_pid);

        if(*L_pid == FALLO) {
            return FALLO;
        }

        printf("[PM] - [%d] Proceso creado \n", pid);

        return pid;

        
    } else if(pid == 0) {

        sprintf(pid_str, "%d", getpid()); // Paso el pid a str para concatenarlo
        strcat(pipe_addr, pid_str); //Address = /tmp/pipe_2180

        int pipe_fd = open(pipe_addr, O_WRONLY);

        if(pipe_fd == FALLO) {
            MYERR(EXIT_FAILURE, "Error al abrir pipe para escritura");
        }

        //Redirigir la salida estandar del proceso a la entrada de la pipe
        dup2(pipe_fd, STDOUT_FILENO);
    
        execvp(comando[0], &comando[0]);

        exit(EXIT_FAILURE);
    } else{
        return FALLO;
    }


}

//----------------------------------------------------------//




//------------------Ejecutar procesos-------------------------//

//Loop principal el cual ejecuta hasta que el sistema se cierre

void ejecutar_procesos(int mm_socket) {

    //Variable auxiliar para optimizar el acceso a shm
    Proceso p;

    int i;

    //Pid del proceso a ejecutar/crear/eliminar/etc
    pid_t pid;

    //Pid del listener del proceso anterior
    pid_t L_pid;

    //Estructura mensaje para mandar a MM creacion del proceso
    Mensaje mensaje = {-1, -1, "", PM};

    while(!sistema_cerrado) {

        for(i = 0; i < PROCESS_MAX; i++) {

            sem_wait(sem);
            p = lista_proceso[i];
            sem_post(sem);

            

            //-----------------CREAR PROCESO------------------------//

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

            //--------------------EJECUTAR PROCESO--------------------------//

            if(p.estado == EJECUTANDO) {
                kill(p.pid, SIGCONT);
                sleep(5);
                kill(p.pid, SIGSTOP);
            }

             //--------------------ELIMINAR PROCESO--------------------------//            

            if(p.estado == ELIMINAR) {
                kill(p.LID, SIGKILL);              
                kill(p.pid, SIGKILL);
                
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

                kill(p.LID, SIGKILL);
                kill(p.pid, SIGKILL);

                sprintf(mensaje.data, "%d-%d", p.pid, FALLO);

                if(send(mm_socket, &mensaje, sizeof(mensaje), MSG_NOSIGNAL) <= 0) {
                    MYERR(EXIT_FAILURE, "Error en el send");
                }

                sem_wait(sem);
                lista_proceso[i].estado = TERMINADO;
                sem_post(sem);
            }

        }
    }
}

//-------------------------------------------------------------------//



//-------------------------CERRAR PROCESOS---------------------------//
//Dado el pid de un proceso cierra de forma correcta al mismo.
void cerrar_proceso(pid_t pid, int tiempo, pid_t LID) {

    int status;
    int estado = 0;

    printf("[PM] Envio signal de terminate a %d \n", pid);
    
    //Mando signal de terminacion al proceso
    if(kill(pid, SIGTERM) == -1) {
        perror("Error en SIGTERM \n");
    }

    //Mando signal de terminacion a su listener
    kill(LID, SIGTERM);

    sleep(tiempo);

    //Si el hijo no cambio su estado en este tiempo devuelve 0
    estado = waitpid(pid, &status, WNOHANG);

    if(estado == 0) {
        printf("[PM] Estas demorando mucho... \n");
        printf("[PM] Envio signal de kill a %d \n", pid);

        if(kill(pid, SIGKILL) == -1) {
            perror("Error en SIGKILL \n");
        }
        kill(LID, SIGKILL);


    //Si no es cero devuelve el pid o error si no se encuentra el hijo
    //De cualquier forma si llega aca es porque el proceso se elimino
    }else{ 
        printf("[PM] Ya se elimino el proceso (%d) y (%d)\n", pid, LID);
    }


}

//---------------------------------------------------------------------//



//------------------------Eliminar Procesos---------------------------//

void eliminar_procesos(void) {

    Proceso p;
    int i;

    for(i = 0; i < PROCESS_MAX; i++) {

        sem_wait(sem);
        p = lista_proceso[i];
        sem_post(sem);

        if(p.estado != TERMINADO) {
            cerrar_proceso(p.pid, 1, p.LID);
        }
    }

}

//---------------------------------------------------------------------//
