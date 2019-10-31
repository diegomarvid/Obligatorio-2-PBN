#ifndef MM_H
#define MM_H

PID crear_proceso(char[] comando, PID rid);

PID eliminar_proceso(PID pid, int *shm);

PID suspender_proceso(PID pid, int *shm);

PID renaudar_proceso(PID pid, int *shm);

int obtener_estado(PID pid, int *shm);

int cerrar_sistema(void);



#endif