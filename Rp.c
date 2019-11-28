#include "funcionesRp.c"


int main(int argc, char const *argv[])
{


    //----------Iniciar interrumpciones----------//
    sigTermSet();

    //---------Buffer para comunicacion por socket-------//
    char buffer[ENTRADA_BUFFSIZE];
    char respuesta[RESPUESTA_BUFFSIZE];
    char salida_buffer[RESPUESTA_BUFFSIZE - 2];

    //--------Obtener socket con consola------------//
    consola_socket = atoi(argv[1]);
   

    //----------Inicializar estructura mensaje con valores default------------//
    Mensaje mensaje = {getpid(), -1, "", RP};

    //--------Conectar con MM-----------//
    mm_socket = sock_connect_un(SOCKET_NAME);

    if( mm_socket == 0 ){
        close(mm_socket);
        MYERR(EXIT_FAILURE, "MM se desconecto \n");
    } else if( mm_socket < 0) {
        close(mm_socket);
        MYERR(EXIT_FAILURE, "Error, no se pudo aceptar conexion. \n");
    }

    printf("[Rp] Escuchando peticiones de consola (%d)\n", getpid());

    //--------Variables select---------//

    //Guardo fd en el array para el select
    monitored_fd_set[0] = consola_socket;
    monitored_fd_set[1] = mm_socket;
    fd_set readfds;



    //-----Variables auxiliares------//
    int r;
    int w;
    int pid;


    //----------------LOOP PRINCIPAL-----------------//

    while(TRUE) {

        //Actualizo los fd a controlar por el select.
        refresh_fd_set(&readfds);

        //Realizo un select el cual nos permite saber si usuario desea realizar una accion o MM nos envia resultados.
        select(get_max_fd() + 1, &readfds, NULL, NULL, NULL);


        //------------------PETICION DE CONSOLA--------------------//

        if(FD_ISSET(consola_socket, &readfds)) {

            //Leo lo recibido por consola
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

            //Convierto buffer a estructura interna del servidor

            conv_to_struct(&mensaje, buffer);

            
            //------------Diferenciar si se engancha a una salida---------------//
            if(mensaje.op == LEER_SALIDA) {

                //En mensaje.data esta el pid en formato string
                char listener_addr[100];             //Direccion para guardar el address de la pipe
                strcpy(listener_addr, L_ADDR);       //Address = /tmp/listener_
                strcat(listener_addr, mensaje.data); //Address = /tmp/listener_2124

                salida_fd = sock_connect_un(listener_addr);

                sscanf(mensaje.data, "%d", &pid);

                //Mando un mensaje sincronico de si se pudo o no engancharse a la salida del proceso
                
                if(salida_fd == FALLO) {
                    sprintf(salida_buffer, "[%d] %s", pid, "No se encontro el proceso\n");
                } else{
                    sprintf(salida_buffer, "[%d] %s", pid, "Exito al engancharse a la salida\n");    
                }

                sprintf(respuesta, "%d-%s", SINCRONICO, salida_buffer);

                if(send(consola_socket, respuesta, sizeof(respuesta) + 1, MSG_NOSIGNAL) < 0) {
                    printf("Error enviando salida del proceso\n");
                }

                //Agrego el fd a la lista para monitorear por el select
                monitored_fd_set[2] = salida_fd;

            } else{

                //------------------ENVIA A MM--------------------//

                //Si recibe de consola y no es engancharse a una salida
                //Rp envia el mensaje a MM

                if (send(mm_socket, &mensaje, sizeof(mensaje), MSG_NOSIGNAL) <= 0){
                    close(mm_socket);
                    MYERR(EXIT_FAILURE, "[Rp] Error en el send \n");
                }

            }

        //-----------------RECIBO RESPUESTA DE MM---------------------//

        } else if(FD_ISSET(mm_socket, &readfds)) {

            //Leo mensaje recibido
            r = recv(mm_socket, &mensaje, sizeof(mensaje), 0);

            if (r == ERROR_CONNECTION){
                close(mm_socket);
                MYERR(EXIT_FAILURE, "[Rp] Error en el recv \n");
            }
            else if (r == END_OF_CONNECTION){
                close(mm_socket);
                MYERR(EXIT_FAILURE, "[Rp] Conexion finalizada con MM \n");
            }

            strcpy(respuesta, "");

            //Si el mensaje que llega de MM lo creo el entonces el id es MM
            //y el mensaje es sicrono.
            //Si el mensaje que llega de MM lo creo PM entonces el id es PM
            //y el mensaje es asicrono.

            //Concateno tipo de comunicacion a la respuesta de MM
            if(mensaje.id == MM){
                sprintf(respuesta, "%d-%s", SINCRONICO, mensaje.data);
            }else if (mensaje.id == PM){
                sprintf(respuesta, "%d-%s", ASINCRONICO, mensaje.data);
            }

            //Envio respuesta a consola
            if (send(consola_socket, respuesta, strlen(respuesta) + 1, MSG_NOSIGNAL) <= 0){
                close(consola_socket);
                MYERR(EXIT_FAILURE, "[Rp] Error en el send \n");
            }

        //---------------------RECIBO SALIDA DE PROCESO---------------------//    

        } else if( FD_ISSET(salida_fd, &readfds) ){

                //Limpio el buffer
                memset(salida_buffer, 0, OUT_BUFFSIZE); 

                //Leo lo que llega
                r = read(salida_fd, salida_buffer, RESPUESTA_BUFFSIZE - 2);

                if(r == ERROR_CONNECTION) {
                    perror("Error en la conexion con el listener");
                    salida_fd = -1;
                } else if(r == END_OF_CONNECTION) {
                    //Si termina de leer el proceso seteo fd a -1 asi no entra
                    //al select
                    salida_fd = -1;
                } else{

                    //Si el mensaje no tiene error concateno tipo de comunicacion
                    //y lo envio a consola
                    sprintf(respuesta, "%d-%s", ASINCRONICO, salida_buffer);
                    w = send(consola_socket, respuesta , r + 2 , MSG_NOSIGNAL);

                }

            
        }

   }

    return 0;
}
