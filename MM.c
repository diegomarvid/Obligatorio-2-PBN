#include "funcionesMM.c"


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

    intitiaze_monitor_fd_set();

    //Lista dinamica para guardar cada Rp con su RID y fd asociado
    lista_fd = dynList_crear();

    //Si hay un socket abierto con el mismo nombre cerralo
    unlink(SOCKET_NAME);

    //Master socket fd para aceptar conexiones
    connection_socket = sock_listen_un(SOCKET_NAME);

    if( connection_socket < 0 ){

        MYERR(EXIT_FAILURE, "Error, no se pudo crear server. \n");

    }

    iniciar_sistema(connection_socket);

    //Crear semaforo
    sem_unlink(SEM_ADDR);    
    sem = sem_open(SEM_ADDR, O_CREAT, 0666, 1);


    //Obtener SHM
    lista_proceso = obtener_shm(OFFSET);


    add_to_monitored_fd_set(connection_socket);


    while (!sistema_cerrado){

        refresh_fd_set(&readfds);

        select(get_max_fd() + 1, &readfds, NULL, NULL, NULL);

        if(FD_ISSET(connection_socket, &readfds)){


            data_socket = sock_open_un(connection_socket);

            if( data_socket == ERROR_CONNECTION ){
                MYERR(EXIT_FAILURE, "Error, no se pudo aceptar conexion. \n");
            }

            add_to_monitored_fd_set(data_socket);
            agregar_nodo(lista_fd, -1, data_socket);


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
                            }

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

    eliminar_sistema();


    return 0;
}
