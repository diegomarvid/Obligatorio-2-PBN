#include "funcionesL.c"


int main(int argc, char const *argv[])
{

    //-------Variables address socket/pipe-------//
    char sock_addr [100];
    char pipe_addr[100];

    //-----------Fd-------------//
    int connection_socket;
    int data_socket;
    int socket_actual;

    //-----Buffer para leer y enviar salida del proceso-----//
    char buffer[RESPUESTA_BUFFSIZE - 2];
    
    //---------Variables para el select y monitoreo---------//
    int proceso_vivo = TRUE;
    int i;
    fd_set readfds;


    //----------------SETEO ADDRESS-----------------//

    strcpy(sock_addr, L_ADDR); //sock_addr = /tmp/listener_
    strcpy(pipe_addr, PIPE_ADDR); //pipe_addr = /tmp/pipe_

    strcat(sock_addr,argv[1]); //sock_addr = /tmp/listener_2048
    strcat(pipe_addr,argv[1]); //pipe_addr = /tmp/pipe_2048

    //Inicializa array de fd para monitoreo en -1
    intitiaze_monitor_fd_set();


    //----------------CONEXION PIPE----------------//

    int pipe_fd = open(pipe_addr, O_RDONLY);
    
    if(pipe_fd == FALLO) {
        MYERR(EXIT_FAILURE, "L no pudo acceder a la pipe");
    }

    //---------------CONEXION SOCKET---------------//

    unlink(sock_addr);

    //Master socket fd para aceptar conexiones
    connection_socket = sock_listen_un(sock_addr);

    if( connection_socket < 0 ){

        MYERR(EXIT_FAILURE, "Error, no se pudo crear server. \n");

    }

    //Agregan fd al array para monitorear en el select
    add_to_monitored_fd_set(connection_socket);
    add_to_monitored_fd_set(pipe_fd);

    //--------------SET INTERRUMPCION----------------//
    sigTermSet();


    //------------------LOOP SELECT------------------//

    //Mientras el proceso a leer este vivo leo su
    //salida y la mando a los Rp que quieren esucharla


    while(proceso_vivo) {


        //Actualizo set de monitoreo para el select
        refresh_fd_set(&readfds);

        select(get_max_fd() + 1, &readfds, NULL, NULL, NULL);

        

        //------------Acepto nuevas conexiones------------//

        if(FD_ISSET(connection_socket, &readfds)){


            data_socket = sock_open_un(connection_socket);

            if( data_socket == ERROR_CONNECTION ){
                MYERR(EXIT_FAILURE, "Error, no se pudo aceptar conexion. \n");
            }

            add_to_monitored_fd_set(data_socket);
            

        //-----------Envio salida a las conexiones----------//

        }else if(FD_ISSET(pipe_fd, &readfds)){

            int w;
            int r;

            memset(buffer, 0, RESPUESTA_BUFFSIZE - 2);

            //-----------Leo salida del proceso------------//

            r = read(pipe_fd, buffer, RESPUESTA_BUFFSIZE - 2);

            if (r == ERROR_CONNECTION){
                perror("Error en la conexion con el listener");
                cerrar_sockets();
                pipe_fd = -1;
                proceso_vivo = FALSE;
            }
            else if (r == END_OF_CONNECTION){       
                proceso_vivo = FALSE;
                cerrar_sockets();
                pipe_fd = -1;
            }
            else{

                //------Mando salida a los Rp conectados------//

                for(i = 0; i < MAX_CLIENTS; i++){

                    socket_actual = monitored_fd_set[i];

                    if(socket_actual != connection_socket && socket_actual != pipe_fd && socket_actual != -1) {
                        w = send(socket_actual, buffer, r, MSG_NOSIGNAL);
                        if(w < 0) {
                            perror("Error en escritura de L");
                            remove_from_monitored_fd_set(socket_actual);
                            close(socket_actual);
                        }
                    }

                } 
            } 

        }
    }
  
    return 0;
}
