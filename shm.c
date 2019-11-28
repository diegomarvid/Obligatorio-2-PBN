#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <semaphore.h>
#include "constantes.h"
#include "shm.h"


//--------------------Inicializar shm----------------------//

void inicializar_shm(Proceso *lista_proceso) {
    

    //p[i] = *(p + i)

    int i;

    for(i = 0; i < TOTAL_PROCESS; i++) {

        lista_proceso[i].RID = INVALIDO;
        lista_proceso[i].LID = TERMINADO;
        lista_proceso[i].pid = INVALIDO;
        lista_proceso[i].estado = TERMINADO;
        
        if(i < OFFSET) {
            strcpy(lista_proceso[i].cmd, "Proceso del sistema");
        } else {
            strcpy(lista_proceso[i].cmd, "Proceso del cliente");
        }

    }


}

//-----------------------------------------------------------//



//-----------------------Crear shm---------------------------//


int crear_shm() {

    //Crear shm

    key_t key = ftok(SHM_ADDR, PROJ_ID);

    if(key == FALLO) {
        printf("Error en el ftok \n");
        return FALLO;
    }

    int id = shmget(key, SHM_SIZE, 0666 | IPC_CREAT);

    printf("\n\nProcesos maximos: %d\n\n", TOTAL_PROCESS);
    
    if(id == FALLO) {
        return FALLO;
    }


    //Inicializar shm
    void *ptr = shmat(id, NULL, 0);

    if(ptr == (void *) FALLO) {
        printf("Error en el attach \n");
        return FALLO;
    }

    inicializar_shm((Proceso *) ptr);


    return id;

}

//-----------------------------------------------------------//



//----------------------Obtener id---------------------------//

int obtener_shm_id() {

    key_t key = ftok(SHM_ADDR, PROJ_ID);

    if(key == FALLO) {
        return FALLO;
    }

    return shmget(key, SHM_SIZE, 0);

}

//-----------------------------------------------------------//


//----------------Obtener puntero shm------------------------//

Proceso *obtener_shm(int offset) {

    key_t key = ftok(SHM_ADDR, PROJ_ID);

    if(key == FALLO) {
        return NULL;
    }

    int id = shmget(key, SHM_SIZE, 0);

    if(id == FALLO) {
        return NULL;
    }

    void *p = shmat(id, NULL, 0);

    if(p == (void *) FALLO) {
        return NULL;
    } else {
        return (Proceso *) p + offset;
    }


}

//-----------------------------------------------------------//