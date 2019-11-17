#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <error.h>
#include <errno.h>
#include "sock.c"
#include "constantes.h"
#include "Rp.h"


void conv_to_struct(Mensaje *mensaje, char buffer[]){

    int op;
    char data[CMD_SIZE];

    if(sscanf(buffer, "%d-%[^\n]s", &op, data) == 0) {
        MYERR(EXIT_FAILURE, "Error en la conversion a mensaje");
    }

    mensaje->RID = getpid();
    mensaje->op = op;
    strcpy(mensaje->data, data);

}



int main(int argc, char const *argv[])
{
    
    //Creo socket
    //int socket = sock_listen(PORT);

    char buffer[BUFFSIZE];

    char string[BUFFSIZE] = "Mensaje del servidor papu";

    printf("[Rp] Se creo un socket entre en el puerto: %d \n", PORT);

    int consola_socket = atoi(argv[1]);

    Mensaje mensaje = {getpid(), -1, ""};

    int mm_socket;

    mm_socket = sock_connect_un();

    if( mm_socket < 0 ){

        MYERR(EXIT_FAILURE, "Error, no se pudo aceptar conexion. \n");

    }

    printf("Connection established with MM \n");
    

    //  if (send(MMsocket, string, strlen(string) + 1, MSG_NOSIGNAL) <= 0)
    //     {
            
    //         close(rdsocket);

    //         MYERR(EXIT_FAILURE, "[Rp] Error en el send \n");
    //     }


    while(1) {
        

        //*********RECIBE CONSOLA********//
        
        int read = recv(consola_socket, buffer, BUFFSIZE, 0);

        if (read < 0)
        {
            close(consola_socket);
        
            MYERR(EXIT_FAILURE, "[Rp] Error en el recv \n");

        } else if(read == 0) {

            close(consola_socket);

            MYERR(EXIT_FAILURE, "[Rp] Conexion finalizada \n");
        }

        printf("[Rp] Recibe de consola: %s \n", buffer);

        //CONVIERTO A ESTRUCTURA INTERNA DEL SERVIDOR
     
        conv_to_struct(&mensaje, buffer);

        
        
        sleep(1);

        //*********MANDA A MM***********//

        if (send(mm_socket, &mensaje, sizeof(mensaje), MSG_NOSIGNAL) <= 0)
        {
            
            close(mm_socket);

            MYERR(EXIT_FAILURE, "[Rp] Error en el send \n");
        }     

        printf("[Rp]->[MM] Manda mensaje \n");
        printf("Op: %d \n", mensaje.op);
        printf("Data: %s \n", mensaje.data);

        //*********RECIBE DE MM***********//

        read = recv(mm_socket, buffer, BUFFSIZE, 0);

        if (read < 0)
        {
            close(mm_socket);
        
            MYERR(EXIT_FAILURE, "[Rp] Error en el recv \n");

        } else if(read == 0) {

            close(mm_socket);

            MYERR(EXIT_FAILURE, "[Rp] Conexion finalizada \n");
        }

        printf("[Rp] Recibe de MM: %s \n", buffer);

        //*********MANDA A Consola***********//

        if (send(consola_socket, buffer, strlen(buffer) + 1, MSG_NOSIGNAL) <= 0)
        {
            
            close(consola_socket);

            MYERR(EXIT_FAILURE, "[Rp] Error en el send \n");
        }

        printf("[Rp]->[C] Manda: %s \n", buffer);

       
       

   }


    return 0;
}
