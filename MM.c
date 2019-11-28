#include "funcionesMM.c"


int main(int argc, char const *argv[]){

    //--------------Socket fd---------------//
    int data_socket;
    int connection_socket;
    int socket_actual = -1;

    //-----------Set para select-----------//
    fd_set readfds;
    int i;
    

    //----------Mensaje para comunicacion-----------//
    Mensaje mensaje;
    
    //---------Nodo para recorrer lista de fd y RID--------//
    Nodo *nodo_fd;

    //---------Inicializar set de monitero de fds----------//
    intitiaze_monitor_fd_set();

    //----------Crear lista dinamica de fd y RID-----------//
    lista_fd = dynList_crear();

   
   //------------------Inicializar socket------------------//
    unlink(SOCKET_NAME);
    
    connection_socket = sock_listen_un(SOCKET_NAME);

    if( connection_socket < 0 ){
        MYERR(EXIT_FAILURE, "Error, no se pudo crear server. \n");
    }

    add_to_monitored_fd_set(connection_socket);

    //-----------------Inicializar sistema------------------//

    iniciar_sistema(connection_socket);


    //--------------Obtener lista de procesos en SHM------------//
    lista_proceso = obtener_shm(OFFSET);


    
    //-----------------Loop de select--------------------//

    //Mientras el sistema no se cierre se reciben peticiones
    //se procesan y se envia su respuesta adecuada.

    while (!sistema_cerrado){

        //----------Actualizar set de monitreo de fd-----------//

        refresh_fd_set(&readfds);

        select(get_max_fd() + 1, &readfds, NULL, NULL, NULL);

        //-----------Aceptar nueva conexion con Rp o PM-------------//

        if(FD_ISSET(connection_socket, &readfds)){


            data_socket = sock_open_un(connection_socket);

            if( data_socket == ERROR_CONNECTION ){
                MYERR(EXIT_FAILURE, "Error, no se pudo aceptar conexion. \n");
            }

            //Agrego el fd a la lista de monitreo
            add_to_monitored_fd_set(data_socket);
            //Agrego el fd a la lista de fd, todavia no se el RID del 
            //proceso que se conecto asi que inicio con -1
            agregar_nodo(lista_fd, -1, data_socket);


        }else{

            socket_actual = -1;

            for(i = 0; i < MAX_CLIENTS; i++){

                if(FD_ISSET(monitored_fd_set[i], &readfds)){

                    socket_actual = monitored_fd_set[i];

                    //------------Recibir y evaluar peticion----------------//

                    int read = recv(socket_actual, &mensaje, sizeof(mensaje), 0);
                    
                    if(read == ERROR_CONNECTION) {
                        eliminar_nodo_fd(lista_fd, socket_actual);
                        close(socket_actual);
                        MYERR(EXIT_FAILURE, "No se pudo leer en socket");

                    } else if(read == END_OF_CONNECTION){
                        eliminar_nodo_fd(lista_fd, socket_actual);
                        remove_from_monitored_fd_set(socket_actual);
                        close(socket_actual);
                    } else {

                        //-------------Categorizar procesamiento-------------//

                        //Si el mensaje es de Rp se ejecuta lo siguiente: 

                        if(mensaje.id == RP) {

                            //Como en el mensaje llega el RID, lo agrego a la 
                            //lista dinamica, para eso primero lo busco con su fd                     
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

                        //Si el mensaje viene de PM ejecuta lo siguiente:

                        } else if(mensaje.id == PM) {

                            //Obtengo el fd del Rp asociado al proceso para enviarle el mensaje.
                            //Se utilizara nodo_fd->fd como el fd indicado para enviar en el socket
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

                                    //Fallo el fork()
                                    
                                    sprintf(mensaje.data, "[%d] %s", pid, "Error en la creacion\n");

                                    mensaje.id = MM;

                                    send(nodo_fd->fd, &mensaje, sizeof(mensaje), MSG_NOSIGNAL);

                                } else{

                                    //Fallo el exec()

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

    //---------------ELIMINACION DEL SISTEMA---------------//

    //Si se llega aca es porque sistema_cerrado es TRUE

    eliminar_sistema();


    return 0;
}
