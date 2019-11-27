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

sem_t *sem;

void inicializar_shm(Proceso *lista_proceso) {

    sem = sem_open(SEM_ADDR, O_CREAT, 0666, 1);

    if(sem == SEM_FAILED) {
        perror("Error en el semaforo");
    }

    //p[i] = *(p + i)

    int i;

    for(i = 0; i < TOTAL_PROCESS; i++) {

        sem_wait(sem);
        lista_proceso[i].RID = INVALIDO;
        lista_proceso[i].LID = TERMINADO;
        lista_proceso[i].pid = INVALIDO;
        lista_proceso[i].estado = TERMINADO;
        

        if(i < 3) {
            strcpy(lista_proceso[i].cmd, "Proceso del sistema");
        } else {
            strcpy(lista_proceso[i].cmd, "Proceso del cliente");
        }
        sem_post(sem);

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

    printf("Procesos totales: %d\nTamano proceso: %ld\nTamano de SHM: %ld\n", TOTAL_PROCESS, sizeof(Proceso), SHM_SIZE);
    
    if(id == FALLO) {
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