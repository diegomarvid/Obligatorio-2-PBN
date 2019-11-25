#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "constantes.h"
#include "shm.h"


void inicializar_shm(Proceso *lista_proceso) {

    //p[i] = *(p + i)

    int i;

    for(i = 0; i < PROCESS_MAX; i++) {

        lista_proceso[i].RID = INVALIDO;
        lista_proceso[i].pid = INVALIDO;
        lista_proceso[i].estado = TERMINADO;
        strcpy(lista_proceso[i].cmd, "Otte se la come");

    }


}


int crear_shm() {

    //Crear shm

    key_t key = ftok(SHM_ADDR, PROJ_ID);

    if(key == FALLO) {
        printf("Error en el ftok \n");
        return FALLO;
    }

    int id = shmget(key, SHM_SIZE, 0666 | IPC_CREAT);

    if(id == FALLO) {
        printf("Error en el shmget \n");
        return FALLO;
    }

    printf("Shm id: %d \n", id);

    //Inicializar shm

    void *ptr = shmat(id, NULL, 0);

    if(ptr == (void *) FALLO) {
        printf("Error en el attach \n");
        return FALLO;
    }

    inicializar_shm((Proceso *) ptr);


    return id;

}



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