#ifndef MM_H
#define MM_H

pid_t crear_proceso(char comando[], PID rid);

pid_t eliminar_proceso(PID pid, int *shm);

pid_t suspender_proceso(PID pid, int *shm);

pid_t renaudar_proceso(PID pid, int *shm);

int obtener_estado(PID pid, int *shm);

int cerrar_sistema(void);



#endif