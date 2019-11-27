#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <error.h>
#include <errno.h>
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h> 
#include <semaphore.h>
#include "constantes.h"
#include "MM.h"
#include "DynList.c"
#include "sock.c"
#include "shm.c"



int monitored_fd_set[MAX_CLIENTS];
Proceso *lista_proceso;
sem_t *sem;
DynList *lista_fd;


/*Remove all the FDs, if any, from the the array*/
static void
intitiaze_monitor_fd_set(){

    int i;

    for(i = 0 ; i < MAX_CLIENTS ; i++) {
        monitored_fd_set[i] = -1;
    }
}


/*Add a new FD to the monitored_fd_set array*/
static void
add_to_monitored_fd_set(int skt_fd){

    int i = 0;

    while (i < MAX_CLIENTS && monitored_fd_set[i] != -1){
        i++;
    }

    if(monitored_fd_set[i] == -1){
        monitored_fd_set[i] = skt_fd;
    }
}


/*Remove the FD from monitored_fd_set array*/
static void
remove_from_monitored_fd_set(int skt_fd){

    int i = 0;

    while (i < MAX_CLIENTS && monitored_fd_set[i] != skt_fd){
        i++;
    }

    if (monitored_fd_set[i] == skt_fd){
        monitored_fd_set[i] = -1;
    }
}


/*Get the numerical max value among all FDs which server
 * is monitoring*/

static int
get_max_fd(){

    int i;
    int max = -1;

    for(i = 0 ; i < MAX_CLIENTS; i++ ){

        if(monitored_fd_set[i] > max){

            max = monitored_fd_set[i];

        }
    }

    return max;
}


/* Clone all the FDs in monitored_fd_set array into
 * fd_set Data structure*/
static void
refresh_fd_set(fd_set *fd_set_ptr){

    FD_ZERO(fd_set_ptr);

    int i;

    for(i = 0; i < MAX_CLIENTS; i++){

        if(monitored_fd_set[i] != -1){

            FD_SET(monitored_fd_set[i], fd_set_ptr);

        }
    }
}

//*******SHM CODE********//

int request_process_space() {

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
            strcpy(mensaje->data,"Error al suspender proceso.\n");
        }else{
            strcpy(mensaje->data,"Exito al suspender proceso.\n");
        }
        mensaje->id = MM;
    }

    if(mensaje->op == RENAUDAR) {
        int pid;
        sscanf(mensaje->data, "%d", &pid);

        if (cambiar_estado_proceso(pid, EJECUTANDO) == FALLO){
            strcpy(mensaje->data,"Error al reanudar proceso.\n");
        }else{
            strcpy(mensaje->data,"Exito al reanudar proceso.\n");
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
}



int main(int argc, char const *argv[]){

    //Variables socket
    int data_socket;
    int connection_socket;
    fd_set readfds;
    int i;
    int socket_actual = -1;

    //Mensaje de comunicacion
    Mensaje mensaje;
    
    //Nodo para utilizar dynlist de fd y RID
    Nodo *nodo_fd;

    sem_unlink(SEM_ADDR);    
    sem = sem_open(SEM_ADDR, O_CREAT, 0666, 1);



    lista_proceso = obtener_shm(OFFSET);

    intitiaze_monitor_fd_set();

    //Lista dinamica para guardar cada Rp con su RID y fd asociado
    lista_fd = dynList_crear();

    //Si hay un socket abierto con el mismo nombre cerralo
    unlink(SOCKET_NAME);

    //Master socket fd para aceptar conexiones
    connection_socket = sock_listen_un();




    if( connection_socket < 0 ){

        MYERR(EXIT_FAILURE, "Error, no se pudo crear server. \n");

    }

    add_to_monitored_fd_set(connection_socket);


    while (TRUE){

        refresh_fd_set(&readfds);

        printf("Waiting on select sys call \n");


        select(get_max_fd() + 1, &readfds, NULL, NULL, NULL);

        if(FD_ISSET(connection_socket, &readfds)){


            data_socket = sock_open_un(connection_socket);

            if( data_socket == ERROR_CONNECTION ){
                MYERR(EXIT_FAILURE, "Error, no se pudo aceptar conexion. \n");
            }

            add_to_monitored_fd_set(data_socket);
            agregar_nodo(lista_fd, -1, data_socket);
            print_dynlist(lista_fd);


        }else{

            socket_actual = -1;

            for(i = 0; i < MAX_CLIENTS; i++){

                if(FD_ISSET(monitored_fd_set[i], &readfds)){

                    socket_actual = monitored_fd_set[i];

                    //Recibo peticion

                    int read = recv(socket_actual, &mensaje, sizeof(mensaje), 0);

                    //Evaluo si se termino la conexion o hay una falla
                    if(read == ERROR_CONNECTION) {

                        close(socket_actual);
                        MYERR(EXIT_FAILURE, "No se pudo leer en socket");

                    } else if(read == END_OF_CONNECTION){
                        eliminar_nodo_fd(lista_fd, socket_actual);
                        remove_from_monitored_fd_set(socket_actual);
                        close(socket_actual);

                    } else {

                        //Dependiendo del identificador del mensaje se realiza una
                        //operacion diferente

                        if(mensaje.id == RP) {

                            //Tengo PID y FD
                            nodo_fd = buscar_nodo_fd(lista_fd, socket_actual);

                            //Agrego RID a la lista dinamica
                            if (nodo_fd != NULL){
                                nodo_fd->data = mensaje.RID;
                                print_dynlist(lista_fd);
                            }

                            printf("[MM] recibe de Rp: \n");
                            printf("Op: %d \n", mensaje.op);
                            printf("Data: %s \n", mensaje.data);
                            printf("id: %d \n", mensaje.id);

                            //Realiza la operacion y genera el mensaje pertinente.
                            ejecutar_operacion(&mensaje, socket_actual);

                            //Envio respuesta a Rp
                            //El caso de creacion no se envia porque el encargado
                            //de crear un proceso es PM y es el responsable de enviar
                            //a MM el resultado de la creacion
                            if(mensaje.op != CREACION) {
                                if(send(socket_actual, &mensaje, sizeof(mensaje), MSG_NOSIGNAL) <= 0){
                                    perror("Error en el send");
                                }
                            }

                            printf("[MM] envia a Rp: \n");
                            printf("Op: %d \n", mensaje.op);
                            printf("Data: %s \n", mensaje.data);
                            printf("id: %d \n", mensaje.id);

                        } else if(mensaje.id == PM) {

                            //Obtengo el fd del Rp asociado al proceso para enviarle el mensaje.
                            nodo_fd = buscar_nodo_data(lista_fd, mensaje.RID);

                            int pid;
                            int status;

                            //PM envia a MM: PID-STATUS para enviar estado de creacion de un proceso
                            sscanf(mensaje.data,"%d-%d", &pid, &status);

                            if(status == FALLO) {

                                //Fallo puede ser por error en el fork()
                                //o error en el exec en una ejecucion
                                //posterior. Por eso se diferencia cuando
                                //el pid es FALLO

                                if(pid == FALLO) {

                                    sprintf(mensaje.data, "[%d] %s", pid, "Error en la creacion\n");

                                    mensaje.id = MM;

                                    send(nodo_fd->fd, &mensaje, sizeof(mensaje), MSG_NOSIGNAL);

                                } else{

                                    sprintf(mensaje.data, "[%d] %s", pid, "Error en la ejecucion\n");

                                    //Se cambia el identificador a PM para despues saber que
                                    //este mensaje es asincrono por un error en ejecucion y
                                    //no en creacion
                                    mensaje.id = PM;

                                    send(nodo_fd->fd, &mensaje, sizeof(mensaje), MSG_NOSIGNAL);

                                }

                            } else {

                                sprintf(mensaje.data, "[%d] %s", pid, "Proceso creado\n");

                                mensaje.id = MM;

                                send(nodo_fd->fd, &mensaje, sizeof(mensaje), MSG_NOSIGNAL);
                            }
                        }
                    }
                }
            }
        }
    }


    return 0;
}
