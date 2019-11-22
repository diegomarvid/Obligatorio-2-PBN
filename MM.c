#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <error.h>
#include <errno.h>
#include "constantes.h"
#include "MM.h"
#include "DynList.c"
#include "sock.c"
#include "shm.c"



int monitored_fd_set[MAX_CLIENTS];
Proceso *lista_proceso;
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
    // for( i = 0 ; i < MAX_CLIENT_SUPPORTED ; i++){

    //     if(monitored_fd_set[i] != -1)
    //         continue;
    //     monitored_fd_set[i] = skt_fd;
    //     break;
    // }

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

    // monitored_fd_set[i] = skt_fd;
    // for(; i < MAX_CLIENT_SUPPORTED; i++){

    //     if(monitored_fd_set[i] != skt_fd)
    //         continue;

    //     monitored_fd_set[i] = -1;
    //     break;
    // }
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
        p = lista_proceso[i];
        if(p.estado == TERMINADO){
            return i;
        }
    }

    return FALLO;

}

int agregar_proceso(char cmd[], int RID){
    
    int indice = request_process_space();

    if(indice != FALLO) {

        lista_proceso[indice].RID = RID;
        lista_proceso[indice].pid = INVALIDO;
        lista_proceso[indice].estado = CREAR;
        strcpy(lista_proceso[indice].cmd, cmd);

        return indice;

    } else{

        return FALLO;
    }
    


}

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



void ejecutar_operacion(Mensaje mensaje) {


    //Crear proceso
    if(mensaje.op == CREACION) {
        if(agregar_proceso(mensaje.data, mensaje.RID) == FALLO) {
            printf("Error al agregar proceso \n");
        }
    }
    if(mensaje.op == ELIMINACION) {
        int pid;
        sscanf(mensaje.data, "%d", &pid);
        cambiar_estado_proceso(pid, TERMINADO);
    }
    if(mensaje.op == SUSPENCION) {
        int pid;
        sscanf(mensaje.data, "%d", &pid);
        cambiar_estado_proceso(pid, SUSPENDIDO);
    }
    if(mensaje.op == RENAUDAR) {
        int pid;
        sscanf(mensaje.data, "%d", &pid);
        cambiar_estado_proceso(pid, EJECUTANDO);
    }



}



int main(int argc, char const *argv[]){

    int data_socket;
    int connection_socket;
    fd_set readfds;
    int i;
    int socket_actual = -1;
    char buffer[BUFFSIZE] = "Soy mm chota grande";
    char buffer2[BUFFSIZE] = "SOY MM CHOTA GRANDE";
    Mensaje mensaje;
    Nodo *nodo_fd;

    lista_proceso = obtener_shm(0);

    intitiaze_monitor_fd_set();

    lista_fd = dynList_crear();


    unlink(SOCKET_NAME);

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

            if( data_socket < 0 ){

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

                    if(read <= 0) {

                        remove_from_monitored_fd_set(socket_actual);
                        close(socket_actual);

                    } else {



                        if(mensaje.id == RP) {

                            //Tengo PID y FD
                            nodo_fd = buscar_nodo_fd(lista_fd, socket_actual);

                            if (nodo_fd != NULL)
                            {
                                nodo_fd->data = mensaje.RID;
                                printf("RID: %d y el nodo tiene: %d", mensaje.RID, nodo_fd->data);
                                print_dynlist(lista_fd);
                            }

                            ejecutar_operacion(mensaje);

                            printf("[MM] recibe de Rp: \n");
                            printf("Op: %d \n", mensaje.op);
                            printf("Data: %s \n", mensaje.data);
                            printf("id: %d \n", mensaje.id);

                            //Procesamiento

                            //Envio respuesta a Rp
                            if(mensaje.op != CREACION) {
                                send(socket_actual, buffer, strlen(buffer) + 1, MSG_NOSIGNAL);
                            }
                            

                            printf("[MM]manda: %s\n", buffer);
                            
                        } else if(mensaje.id == PM) {

                            nodo_fd = buscar_nodo_data(lista_fd, mensaje.RID);

                            printf("RID PM: %d \n", nodo_fd->data);
                            printf("FD RID PM: %d \n", nodo_fd->fd);
                            
                            int pid;
                            int status;

                            sscanf(mensaje.data,"%d-%d", &pid, &status);
                            if(status == FALLO) {

                                if(pid == -1) {
                                    printf("[%d] Error en la creacion \n", pid);
                                } else{
                                    printf("[%d] Error en proceso \n", pid);
                                    send(nodo_fd->fd, buffer2, strlen(buffer2) + 1, MSG_NOSIGNAL);

                                }
                                
                            } else {
                                printf("[%d] Proceso creado \n", pid);
                                send(nodo_fd->fd, buffer2, strlen(buffer2) + 1, MSG_NOSIGNAL);
                            }

                            
                            

                        }           

                    }    
                    //remove_from_monitored_fd_set(socket_actual);
                }   

            }

        }

    }

    // printf("Me cree de forma correcta:D\n");


    // int sock_Rp;

    // sock_Rp = sock_open_un(connection_socket);

    // if( sock_Rp < 0 ){

    //     MYERR(EXIT_FAILURE, "Error, no se pudo aceptar conexion. \n");

    // }

    // printf("Se me conecto un Rp y su socket es %d \n",sock_Rp);


       
    //ejemplo(print);
    
    return 0;
}
