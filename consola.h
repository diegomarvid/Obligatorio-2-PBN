#ifndef CONSOLA_H
#define CONSOLA_H

void refresh_fd_set(fd_set *fd_set_ptr);

void sigIntHandler(int signum, siginfo_t *info, void *ucontext);

void sigIntSet(void);

void desplegar_menu(void);

int crear_mensaje(int opcion, char msg[]);

void transimitir_mensaje(int sockfd, char mensaje[], char respuesta[]);


#endif