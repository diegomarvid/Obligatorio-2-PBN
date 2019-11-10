#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <error.h>
#include <errno.h>
#include "sock.c"
#include "constantes.h"
#include "Rp.h"



int main(int argc, char const *argv[])
{
    
    //Creo socket
    //int socket = sock_listen(PORT);

    char mensaje[BUFFSIZE];

    char string[BUFFSIZE] = "Mensaje del servidor papu";

    printf("Se creo un socket entre en el puerto: %d \n", PORT);

    int rdsocket = atoi(argv[1]);

    while(1) {
        
        //Acepto conexion
        //int rdsocket = sock_open(socket);

        //Envio mensaje de bienvenida
        if (recv(rdsocket, mensaje, BUFFSIZE, 0) <= 0)
        {
            close(rdsocket);
        
            MYERR(EXIT_FAILURE, "Error en el recv \n");
        }

        printf("[Rp] Recibe: %s \n", mensaje);
        
        sleep(1);

        if (send(rdsocket, string, strlen(string) + 1, MSG_NOSIGNAL) <= 0)
        {
            
            close(rdsocket);

            MYERR(EXIT_FAILURE, "Error en el send \n");
        }

        printf("[Rp] Manda: %s \n", string);
       
       

   }


    return 0;
}
