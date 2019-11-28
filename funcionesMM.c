#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <error.h>
#include <errno.h>
#include <fcntl.h>           
#include <sys/stat.h> 
#include <sys/wait.h>
#include <sys/types.h>
#include <semaphore.h>
#include "constantes.h"
#include "MM.h"
#include "DynList.c"
#include "sock.c"
#include "shm.c"


//----------------------VARIABLES GLOBALES------------------------//


//---------Array de monitoreo de fd para el select----------//
int monitored_fd_set[MAX_CLIENTS];

//---------Lista de procesos para recorrer la shm-----------//
Proceso *lista_proceso;

//----------Semaforo----------//
sem_t *sem;

//----------Lista de fd con su RID asociado--------------//
DynList *lista_fd;

//----------Variable para evaluar la eliminacion del sistema---------//
volatile int sistema_cerrado = FALSE;

//---------Socket fd para aceptar conexiones-----------//
int connection_socket;


//-----------------------FUNCIONES-----------------------------//


//----------Inicializar array de monitoreo en -1--------------//
void intitiaze_monitor_fd_set(void){

    int i;

    for(i = 0 ; i < MAX_CLIENTS ; i++) {
        monitored_fd_set[i] = -1;
    }
}

//-------------------------------------------------------------//



//---------------Agrega fd en posicion libre------------------//
void add_to_monitored_fd_set(int skt_fd){

    int i = 0;

    while (i < MAX_CLIENTS && monitored_fd_set[i] != -1){
        i++;
    }

    if(monitored_fd_set[i] == -1){
        monitored_fd_set[i] = skt_fd;
    }
}

//------------------------------------------------------------//




//-----------------Eliminar fd de la lista--------------------//
void remove_from_monitored_fd_set(int skt_fd){

    int i = 0;

    while (i < MAX_CLIENTS && monitored_fd_set[i] != skt_fd){
        i++;
    }

    if (monitored_fd_set[i] == skt_fd){
        monitored_fd_set[i] = -1;
    }
}


//------------------------------------------------------------//



//------------------Obtener maximo del array------------------//
int get_max_fd(){

    int i;
    int max = -1;

    for(i = 0 ; i < MAX_CLIENTS; i++ ){

        if(monitored_fd_set[i] > max){

            max = monitored_fd_set[i];

        }
    }

    return max;
}


//------------------------------------------------------------//



//---------------Actualizar set de monitero-------------------//
void refresh_fd_set(fd_set *fd_set_ptr){

    FD_ZERO(fd_set_ptr);

    int i;

    for(i = 0; i < MAX_CLIENTS; i++){

        if(monitored_fd_set[i] != -1){

            FD_SET(monitored_fd_set[i], fd_set_ptr);

        }
    }
}


//------------------------------------------------------------//



//--------------Obtener espacio libre de SHM------------------//

int request_process_space(void) {

    Proceso p;
    int i;

    for(i = 0; i < PROCESS_MAX; i++) {
        sem_wait(sem);
        p = lista_proceso[i];
        sem_post(sem);
        if(p.estado == TERMINADO){
            return i;
        }
    }

    return FALLO;

}

//------------------------------------------------------------//



//--------------Obtener estado de proceso en shm--------------//

int obtener_estado(pid_t pid) {

  Proceso p;  
  int i;

  for(i = 0; i < PROCESS_MAX; i++) {
      
      sem_wait(sem);
      p = lista_proceso[i];
      sem_post(sem);
      
      if(p.pid == pid && p.estado != TERMINADO) {       
        return p.estado;
      }
  }

  return INVALIDO;
}

//------------------------------------------------------------//



//--------------Agregar proceso para crear--------------------//

int agregar_proceso(char cmd[], int RID){

    int indice = request_process_space();

    if(indice != FALLO) {

        sem_wait(sem);

        lista_proceso[indice].RID = RID;
        lista_proceso[indice].LID = INVALIDO;
        lista_proceso[indice].pid = INVALIDO;
        lista_proceso[indice].estado = CREAR;
        strcpy(lista_proceso[indice].cmd, cmd);

        sem_post(sem);

        return indice;

    } else{

        return FALLO;
    }

}

//------------------------------------------------------------//



//----------------Cambiar estado de proceso-------------------//

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

//------------------------------------------------------------//



//---------------Formatear estado a string--------------------//

void formatear_estado(Mensaje *mensaje, pid_t pid, int estado) {


    if(estado == INVALIDO) {
      sprintf(mensaje->data, "[%d] %s", pid, "No se encontro el proceso ");
    } else if(estado == CREAR) {
      sprintf(mensaje->data, "[%d] %s", pid, "En proceso de creacion ");
    } else if(estado == EJECUTANDO) {
      sprintf(mensaje->data, "[%d] %s", pid, "Ejectuando ");
    } else if(estado == SUSPENDIDO) {
      sprintf(mensaje->data, "[%d] %s", pid, "Suspendido ");
    }

    mensaje->id = MM;

}

//------------------------------------------------------------//



//-----------------Obtener lista dado RID---------------------//

void obtener_lista(Mensaje *mensaje){

    char lista_str[DATA_SIZE] = "";
    char aux_str[ENTRADA_BUFFSIZE + 1] = "";
    int i;
    int cantidad_encontrados = 0;
    Proceso p;
    char end = (char) 27;

    for(i = 0; i < PROCESS_MAX; i++) {

        sem_wait(sem);
        p = lista_proceso[i];
        sem_post(sem);

        if(p.RID == mensaje->RID && p.estado != TERMINADO && p.estado != INVALIDO) {
            cantidad_encontrados++;
            sprintf(aux_str,"%d-%s", p.pid ,p.cmd);
            strncat(aux_str,&end,1);
            strcat(lista_str,aux_str);
        }
    }


    if (cantidad_encontrados > 0){
        strcpy(mensaje->data, lista_str);
    }
    else{
        strcpy(mensaje->data, "No se encontro ningun proceso.\n");
    }

    mensaje->id = MM;
}

//------------------------------------------------------------//



//--------------Ejecutar todas las operaciones----------------//

void ejecutar_operacion(Mensaje *mensaje, int socket_actual) {


    //Crear proceso
    if(mensaje->op == CREACION) {

        mensaje->id = MM;
        if(agregar_proceso(mensaje->data, mensaje->RID) == FALLO) {
            strcpy(mensaje->data,"Error al crear, capacidad maxima de procesos alcanzada.\n");
            send(socket_actual, mensaje, sizeof(*mensaje), MSG_NOSIGNAL);
        }

    }
    if(mensaje->op == ELIMINACION) {
        int pid;
        //En data viene el PID del proceso a eliminar en formato str
        sscanf(mensaje->data, "%d", &pid);

        if (cambiar_estado_proceso(pid, ELIMINAR) == FALLO){
            sprintf(mensaje->data,"[%d] Error al eliminar proceso.\n", pid);
        }else{
            sprintf(mensaje->data,"[%d] Proceso terminado.\n", pid);
        }
        mensaje->id = MM;
    }
    if(mensaje->op == SUSPENCION) {
        int pid;
        sscanf(mensaje->data, "%d", &pid);

        if (cambiar_estado_proceso(pid, SUSPENDIDO) == FALLO){
            sprintf(mensaje->data,"[%d] Error al suspender proceso.\n", pid);
        }else{
            sprintf(mensaje->data,"[%d] Proceso suspendido.\n", pid);
        }

        mensaje->id = MM;
    }

    if(mensaje->op == RENAUDAR) {
        int pid;
        sscanf(mensaje->data, "%d", &pid);

        if (cambiar_estado_proceso(pid, EJECUTANDO) == FALLO){
            sprintf(mensaje->data,"[%d] Error al reanudar proceso.\n", pid);
        }else{
            sprintf(mensaje->data,"[%d] Proceso reanudado.\n", pid);
        }

        mensaje->id = MM;
    }

    if(mensaje->op == ESTADO){
        int pid;
        sscanf(mensaje->data, "%d", &pid);

        int estado = obtener_estado(pid);
        formatear_estado(mensaje, pid, estado);
    }

    if(mensaje->op == LISTA){
        obtener_lista(mensaje);
    }

    if(mensaje->op == CERRAR_SISTEMA){
        sistema_cerrado = TRUE;
    }
}

//------------------------------------------------------------//



//--------------INICIALIZACION DEL SISTEMA--------------------//

void iniciar_sistema(int connection_socket) {

    //--------------Crear shm----------------//

    if(remove(SHM_ADDR) == FALLO) {
        printf("Archivo no estaba creado \n");
    }

     if(fopen(SHM_ADDR, "w") ==  NULL) {
         MYERR(EXIT_FAILURE, "Error en la creacion del archivo de shm");
     }
    
    if(crear_shm() == FALLO){
        MYERR(EXIT_FAILURE, "Error en la creacion de shm");
    }

    //-----------Obtener puntero shm------------//

    Proceso *procesos_sistema = obtener_shm(0);

    //-------------Crear semaforo-----------//
    sem_unlink(SEM_ADDR);    
    sem = sem_open(SEM_ADDR, O_CREAT, 0666, 1);

    //---------Guardar estructura en shm----------//
    procesos_sistema[0].pid = getpid();
    procesos_sistema[0].RID = INVALIDO;
    procesos_sistema[0].LID = INVALIDO;
    strcpy(procesos_sistema[0].cmd, "MM");

    //------------Crear PM------------//

    pid_t pm_pid = fork();

    if(pm_pid == FALLO) {
        perror("Error al crear PM");
    } else if(pm_pid == 0) {

        //Cierro los sockets de MM
        close(connection_socket);

        //Tiempo para que MM se ejecute y abra conexiones para esperar
        sleep(2);

        execlp("./PM", "PM", NULL);

        MYERR(EXIT_FAILURE, "Error al ejecutar PM");

    }

    sleep(2);

    //---------Guardar estructura en shm----------//
    sem_wait(sem);
    procesos_sistema[1].pid = pm_pid;
    procesos_sistema[1].RID = INVALIDO;
    procesos_sistema[1].LID = INVALIDO;
    strcpy(procesos_sistema[1].cmd, "PM");
    sem_post(sem);

    //------------Crear R------------//

    pid_t r_pid = fork();

    if(r_pid == FALLO) {
        perror("Error al crear R");
    } else if(r_pid == 0) {

        //Cierro los sockets de MM
        close(connection_socket);
        execlp("./R", "R", NULL);

        MYERR(EXIT_FAILURE, "Error al ejecutar R");

    }

    //---------Guardar estructura en shm----------//
    sem_wait(sem);
    procesos_sistema[2].pid = r_pid;
    procesos_sistema[2].RID = INVALIDO;
    procesos_sistema[2].LID = INVALIDO;
    strcpy(procesos_sistema[2].cmd, "R");
    sem_post(sem);



}
//------------------------------------------------------------//



//--------------------Cerrar proceso--------------------------//

void cerrar_proceso(pid_t pid, int tiempo) {

    int status;
    int estado;

    printf("*[MM] Envio signal de terminate a %d \n\n", pid);
    

    if(kill(pid, SIGTERM) == -1) {
        perror("Error en SIGTERM \n");
    }

    sleep(tiempo);

    estado = waitpid(pid, &status, WNOHANG);

    if(estado == 0) {
    
        printf("[MM] Estas demorando mucho... \n");
        printf("[MM] Envio signal de kill a %d \n", pid);

        if(kill(pid, SIGKILL) == -1) {
            perror("Error en SIGKILL \n");
        }
        
    } else if(estado == -1) {
        //Si SIGTERM anduvo da este error, deberia manejarlo distinto
        //perror("Error en waitpid WNOHANG \n");
        printf("\n*[MM] Ya se elimino el proceso (%d)\n\n", pid);
    } else {
        printf("\n*[MM] Ya se elimino el proceso (%d)\n\n", pid);
    }



}

//------------------------------------------------------------//


//--------------Eliminar lista dinamica de Rp-----------------//

void cerrar_lista_fd(void) {

    printf("[R] Cerrando Rp...\n");

    Nodo *actual = lista_fd->head;
    Nodo *aux = actual;
    
    while(actual != NULL){   
        aux = actual;
        actual = actual->next;
        free(aux);
    }

    //Despues de eliminar todos los nodos se elimina el espacio
    //de memoria de la lista
    free(lista_fd);

}

//------------------------------------------------------------//


//--------------------ELIMINAR SISTEMA------------------------//

void eliminar_sistema(void) {


    //Obtiene puntero a shm para recorrer los procesos del sistema
    Proceso *procesos_sistema = obtener_shm(0);

    
    pid_t pid;
    int i;

    printf("\n\n------ELIMINACION DEL SISTEMA------- \n\n");

    //---------------Eliminar R--------------//

    //Obtengo pid de R
    sem_wait(sem);
    pid = procesos_sistema[2].pid;
    sem_post(sem);

    cerrar_proceso(pid, 4);

    //---------------Eliminar PM--------------//

    //Obtengo pid de PM
    sem_wait(sem);
    pid = procesos_sistema[1].pid;
    sem_post(sem);

    cerrar_proceso(pid, 10);

    //---------------Eliminar Semaforo, sockets y shm--------------//

    //Semaforo
    sem_close(sem);
    sem_unlink(SEM_ADDR);

    //SHM
    int id = obtener_shm_id();
    shmdt(procesos_sistema);
    shmctl(id, IPC_RMID, 0);

    //Sockets
    for(i = 0; i < MAX_CLIENTS; i++){
        if(monitored_fd_set[i] != -1){
            close(monitored_fd_set[i]);
        }
    }

    unlink(SOCKET_NAME); 
    

}

//------------------------------------------------------------//