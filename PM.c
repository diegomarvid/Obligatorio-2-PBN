
#include "funcionesPM.c"

//*******Funciones de signals********//

void sigChildHandler(int signum, siginfo_t *info, void *ucontext ) {

    if(sistema_cerrado == TRUE) {
        return;
    }

    int status;
    pid_t pid;

    

    pid = waitpid(-1, &status, 0);

    if(pid == FALLO) {
      printf("Fallo el waitpid \n");
    } else {

        char pipe_addr[100];          //Direccion para guardar el address de la pipe
        strcpy(pipe_addr, PIPE_ADDR); //Address = /tmp/pipe_
        char pid_str[20];
        sprintf(pid_str, "%d", pid); // Paso el pid a str para concatenarlo
        strcat(pipe_addr, pid_str);  //Address = /tmp/pipe_2180

        unlink(pipe_addr);
    }

    int semval;
    int interrumpi_sem = FALSE;
    sem_getvalue(sem,&semval);


    //printf("El semaforo tiene un valor:%d\n",semval);

    //printf("[Signal] status: %d \n", status);

    if(semval == 0){
        sem_post(sem);
        interrumpi_sem = TRUE;
    }

    

    if(status == EXIT_SUCCESS) {
      cambiar_estado_proceso(pid, ELIMINAR);
      
      printf("[%d] Proceso terminado \n", pid);
    } else {
      cambiar_estado_proceso(pid, INVALIDO);
      printf("[%d] Proceso invalido \n", pid);
    }

    sem_getvalue(sem,&semval);

    if(semval == 1 && interrumpi_sem) {
        sem_wait(sem);
    }

   //printf("El semaforo tiene un valor:%d\n",semval);

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

void sigTermHandler(int signum, siginfo_t *info, void *ucontext) {

    sistema_cerrado = TRUE;
    printf("[PM] Me estoy autoeliminando...\n");

}


void sigTermSet() {
    struct sigaction action, oldaction;

    action.sa_sigaction = sigTermHandler; //Funcion a llamar
    sigemptyset(&action.sa_mask);
    sigfillset(&action.sa_mask); //Bloqueo todas la seniales
    action.sa_flags = SA_SIGINFO;
    action.sa_restorer = NULL;

    sigaction(SIGTERM, &action, &oldaction);
}









int main(int argc, char const *argv[])
{


    //Seteo interrupciones para Child
    sigChildSet();
    sigTermSet();

    sem = sem_open(SEM_ADDR, O_CREAT, 0666, 1);

    int mm_socket;
    mm_socket = sock_connect_un(SOCKET_NAME);

    if( mm_socket < 0 ){
        MYERR(EXIT_FAILURE, "Error, no se pudo aceptar conexion. \n");
    }

    printf("Connection established with MM \n");

    lista_proceso = obtener_shm(OFFSET);

    ejecutar_procesos(mm_socket);

    eliminar_procesos();

    sem_close(sem);
    shmdt((void*)obtener_shm(0));
    


    return 0;
}
