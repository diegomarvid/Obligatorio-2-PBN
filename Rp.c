#include "funcionesRp.c"


int main(int argc, char const *argv[])
{

    sigTermSet();

    //Creo socket
    //int socket = sock_listen(PORT);

    char buffer[ENTRADA_BUFFSIZE];
    char respuesta[RESPUESTA_BUFFSIZE];

    //Obtengo el valor del socket que conecta a Rp con su consola.
    consola_socket = atoi(argv[1]);
   

    //Inicializo la estructura mensaje con valores default.
    Mensaje mensaje = {getpid(), -1, "", RP};

    //Conecto a Rp con MM.
    mm_socket = sock_connect_un(SOCKET_NAME);

    //Manejo de error en conexion con MM
    if( mm_socket == 0 ){
        close(mm_socket);
        MYERR(EXIT_FAILURE, "MM se desconecto \n");
    } else if( mm_socket < 0) {
        close(mm_socket);
        MYERR(EXIT_FAILURE, "Error, no se pudo aceptar conexion. \n");
    }

    printf("[Rp] Escuchando peticiones de consola (%d)\n", getpid());

    //*****Variables select*******//

    //Guardo fd en el array para el select
    monitored_fd_set[0] = consola_socket;
    monitored_fd_set[1] = mm_socket;
    fd_set readfds;



    //******Variables socket******/
    
    int r;
    char salida_buffer[RESPUESTA_BUFFSIZE - 2];
    int pid;

    while(TRUE) {

        //Actualizo los fd a controlar por el select.
        refresh_fd_set(&readfds);

        //Realizo un select el cual nos permite saber si usuario desea realizar una accion o MM nos envia resultados.
        select(get_max_fd() + 1, &readfds, NULL, NULL, NULL);

        if(FD_ISSET(consola_socket, &readfds)) {

            //*********RECIBE CONSOLA********//

            r = recv(consola_socket, buffer, ENTRADA_BUFFSIZE, 0);

            if (r == ERROR_CONNECTION){
                close(consola_socket);
                close(mm_socket);
                MYERR(EXIT_FAILURE, "[Rp] Error en el recv \n");
            }
            else if (r == END_OF_CONNECTION){
                close(consola_socket);
                close(mm_socket);
                MYERR(EXIT_FAILURE, "[Rp] Conexion finalizada con Consola\n");
            }

            //CONVIERTO A ESTRUCTURA INTERNA DEL SERVIDOR

            conv_to_struct(&mensaje, buffer);

            //Diferenciar si se quiere enganchar a la salida de otro proceso

            if(mensaje.op == LEER_SALIDA) {

                //En mensaje.data esta el pid en formato string
                char listener_addr[100];             //Direccion para guardar el address de la pipe
                strcpy(listener_addr, L_ADDR);       //Address = /tmp/listener_
                strcat(listener_addr, mensaje.data); //Address = /tmp/listener_2124

                salida_fd = sock_connect_un(listener_addr);

                sscanf(mensaje.data, "%d", &pid);

                
                if(salida_fd == FALLO) {
                    sprintf(salida_buffer, "[%d] %s", pid, "No se encontro el proceso\n");
                } else{
                    sprintf(salida_buffer, "[%d] %s", pid, "Exito al engancharse a la salida\n");    
                }

                sprintf(respuesta, "%d-%s", SINCRONICO, salida_buffer);

                send(consola_socket, respuesta, sizeof(respuesta) + 1, MSG_NOSIGNAL);

                monitored_fd_set[2] = salida_fd;

            } else{

                //*********MANDA A MM***********//

                if (send(mm_socket, &mensaje, sizeof(mensaje), MSG_NOSIGNAL) <= 0){
                    close(mm_socket);
                    MYERR(EXIT_FAILURE, "[Rp] Error en el send \n");
                }

            }
        } else if(FD_ISSET(mm_socket, &readfds)) {

            //*********RECIBE DE MM***********//
            //Rebice mensaje y lo formatea.

            r = recv(mm_socket, &mensaje, sizeof(mensaje), 0);

            if (r == ERROR_CONNECTION){
                close(mm_socket);
                MYERR(EXIT_FAILURE, "[Rp] Error en el recv \n");
            }
            else if (r == END_OF_CONNECTION){
                close(mm_socket);
                MYERR(EXIT_FAILURE, "[Rp] Conexion finalizada con MM \n");
            }

            //*********MANDA A Consola***********//
            //Formatear mensaje para consola.

            //A Rp le llega de la forma PID-RESPUESTA_STR
            strcpy(respuesta, "");

            if(mensaje.id == MM){
                sprintf(respuesta, "%d-%s", SINCRONICO, mensaje.data);
            }else if (mensaje.id == PM){
                sprintf(respuesta, "%d-%s", ASINCRONICO, mensaje.data);
            }

            //printf("%s\n",buffer);

            if (send(consola_socket, respuesta, strlen(respuesta) + 1, MSG_NOSIGNAL) <= 0){
                close(consola_socket);
                MYERR(EXIT_FAILURE, "[Rp] Error en el send \n");
            }

        } else if( FD_ISSET(salida_fd, &readfds) ){

                int w;    
 
                memset(salida_buffer, 0, OUT_BUFFSIZE); 

                r = read(salida_fd, salida_buffer, RESPUESTA_BUFFSIZE - 2);

                if(r == ERROR_CONNECTION) {
                    perror("Error en la conexion con el listener");
                    salida_fd = -1;
                } else if(r == END_OF_CONNECTION) {
                    //perror("Se termino de leer el proceso");
                    salida_fd = -1;
                } else{
                    sprintf(respuesta, "%d-%s", ASINCRONICO, salida_buffer);
                    w = send(consola_socket, respuesta , r + 2 , MSG_NOSIGNAL);

                }

            
        }

   }

    return 0;
}
