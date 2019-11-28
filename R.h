#ifndef R_H
#define R_H

void sigChildHandler(int signum, siginfo_t *info, void *ucontext);

void sigChildSet(void);

void sigTermHandler(int signum, siginfo_t *info, void *ucontext);

void sigTermSet(void);

void cerrar_proceso(pid_t pid, int tiempo);

pid_t crear_Rp(int sockfd, int socket); // Asignar conexion a R' con consola

void cerrar_lista_Rp(void); //Eliminar todos los R' al cerrar el sistema

#endif
