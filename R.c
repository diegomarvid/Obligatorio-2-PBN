#include "funcionesR.c"


//--------------CODIGO DE R----------------//
int main(int argc, char const *argv[])
{

    sigChildSet();
    sigTermSet();

    fd_set readfds;

    //Inicializo la lista de los Rp.
    lista_Rp = dynList_crear();


    //Creo un socket para conectar a las consolas con el sistema.
    connection_socket = sock_listen_in(PORT); 

    while(!sistema_cerrado){

        //Refresh fdset
        FD_ZERO(&readfds);
        FD_SET(connection_socket, &readfds);

        select(connection_socket + 1, &readfds, NULL, NULL, NULL);

        if(FD_ISSET(connection_socket, &readfds)) {

            //----------Acepto conexion y derivo a Rp--------------//

            //printf("Esperando conexion..\n");

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

       // printf("No veo nueva conexion \n"); 
        

    }

    cerrar_lista_Rp();

    close(connection_socket);


    return 0;
}

