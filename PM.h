#ifndef PM_H
#define PM_H

void sigChildHandler(int signum, siginfo_t *info, void *ucontext);

void sigChildSet(void);

void sigTermHandler(int signum, siginfo_t *info, void *ucontext);

void sigTermSet(void);

void str_split(char *build_string[], char string[], char *delim);

int cambiar_estado_proceso(pid_t pid, int estado);

int crear_name_pipe(char *pipe_addr);

void crear_listener(char *pid_str, int *L_pid);

pid_t crear_proceso(char cmd[], int *L_pid);

void ejecutar_procesos(int mm_socket);

void cerrar_proceso(pid_t pid, int tiempo, pid_t LID);

void eliminar_procesos(void);


#endif