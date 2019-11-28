#ifndef PM_H
#define PM_H


void str_split(char *build_string[], char string[], char *delim);

int cambiar_estado_proceso(pid_t pid, int estado);

int crear_name_pipe(char *pipe_addr);

void crear_listener(char *pid_str, int *L_pid);

pid_t crear_proceso(char cmd[], int *L_pid);

void ejecutar_procesos(int mm_socket);

void cerrar_proceso(pid_t pid, int tiempo);

void eliminar_procesos(void);


#endif