#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <error.h>
#include <errno.h>
#include "sock.c"
#include "constantes.h"

#define PORT 3045

int main(){
char mensaje[BUFFSIZE] = "Hola que tal";

int socket = sock_connect(SERVERHOST, PORT);

if(send(socket, mensaje, (strlen(mensaje) + 1), MSG_NOSIGNAL) == -1) {
    MYERR(EXIT_FAILURE, "Error en el send \n");
}

printf("[Consola] Manda: %s \n", mensaje);

if(recv(socket, mensaje, BUFFSIZE, 0) == -1) {
    MYERR(EXIT_FAILURE, "Error en el recv \n");
}

printf("[Consola] Recibe: %s \n", mensaje);



sleep(1);

return 0;

}