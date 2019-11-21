#ifndef SHM_H
#define SHM_H

void inicializar_shm(Proceso *lista_proceso);

int crear_shm();

Proceso *obtener_shm(int offset);



#endif