#include "funcionesR.c"


int main(int argc, char const *argv[])
{

    //------------Inicializar interrumpciones--------------//
    sigChildSet();
    sigTermSet();

    //------------Set de fd para el select-----------------//
    fd_set readfds;

    //-------------Inicializar lista de Rp-----------------//
    lista_Rp = dynList_crear();


    //-------------Obtener socket de conexion--------------//
    connection_socket = sock_listen_in(PORT); 

    printf("[R] Escuchando en IP: %s:%d \n", SERVERHOST, PORT);


    //----------------Aceptar conexiones-------------------//

    //La razon del select es para que cuando la variable
    //sistema_cerrado cambie de valor, el programa no quede
    //trancado en el accept y pueda volver al while y terminar
    //correctamente.

    while(!sistema_cerrado){

        //Actualizar set de fd para el select
        FD_ZERO(&readfds);
        FD_SET(connection_socket, &readfds);

        select(connection_socket + 1, &readfds, NULL, NULL, NULL);


        //Si llega una conexion se acepta

        if(FD_ISSET(connection_socket, &readfds)) {

            //----------Acepto conexion y derivo a Rp--------------//
            int sock_Rp = sock_open_in(connection_socket);

            if (sock_Rp == ERROR_CONNECTION){
                MYERR(EXIT_FAILURE, "Error, no se pudo aceptar conexion \n");
            }
            else{                 
                if (crear_Rp(sock_Rp, connection_socket) == FALLO){         
                    MYERR(EXIT_FAILURE, "Error, R no pudo crear Rp \n");
                }
               
            }
        }
        
    }

    //Cerrar Rp antes de morir
    cerrar_lista_Rp();
    //Terminar de aceptar conexiones
    close(connection_socket);
    

    return 0;
}

