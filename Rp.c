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

    int rdsocket = atoi(argv[1]);
    Mensaje mensaje = {getpid(), -1, ""};

    while(1) {
        
        
        int read = recv(rdsocket, buffer, BUFFSIZE, 0);

        if (read < 0)
        {
            close(rdsocket);
        
            MYERR(EXIT_FAILURE, "[Rp] Error en el recv \n");
        } else if(read == 0) {

            close(rdsocket);

            MYERR(EXIT_FAILURE, "[Rp] Conexion finalizada \n");
        }
   
        //Convierto string de consola en estructura mensaje
        //para poder comunicar con el sistema de forma eficiente
        conv_to_struct(&mensaje, buffer);

        printf("Op: %d \n", mensaje.op);
        printf("Data: %s \n", mensaje.data);
        
        sleep(1);

        if (send(rdsocket, string, strlen(string) + 1, MSG_NOSIGNAL) <= 0)
        {
            
            close(rdsocket);

            MYERR(EXIT_FAILURE, "[Rp] Error en el send \n");
        }

        printf("[Rp] Manda: %s \n", string);
       
       

   }


    return 0;
}
