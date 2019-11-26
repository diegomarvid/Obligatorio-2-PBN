#include <stdio.h>
#include <stdlib.h>
#include "shm.c"
#include "constantes.h"
#include <error.h>
#include <errno.h>

int main(int argc, char const *argv[])
{

    if(remove(SHM_ADDR) == FALLO) {
        printf("Archivo no estaba creado \n");
    }

     if(fopen(SHM_ADDR, "w") ==  NULL) {
         MYERR(EXIT_FAILURE, "Error en la creacion del archivo de shm");
     }
    
    if(crear_shm() == FALLO){
        MYERR(EXIT_FAILURE, "Error en la creacion de shm");
    }

    printf("Shm creada con exito \n");

    Proceso *p = obtener_shm(0);

    printf("Proceso cmd sistema: %s \n", p[2].cmd);
    printf("Proceso cmd cliente: %s \n", p[3].cmd);

    


    return 0;
}
