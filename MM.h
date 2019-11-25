#ifndef MM_H
#define MM_H

pid_t crear_proceso(char comando[], pid_t rid);

pid_t eliminar_proceso(pid_t pid);

pid_t suspender_proceso(char *data);

pid_t renaudar_proceso(pid_t pid, int *shm);

int obtener_estado(pid_t pid);

int cerrar_sistema(void);



#endif
