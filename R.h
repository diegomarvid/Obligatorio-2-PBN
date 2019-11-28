#ifndef R_H
#define R_H

//Rp -> R prima

pid_t crear_Rp(int sockfd,int socket); // Asignar conexion a R' con consola

void cerrar_lista_Rp(void); //Eliminar todos los R' al cerrar el sistema

void eliminar_Rp(void); //Eliminar R' de la lista al cierre de conexion

void sigChildHandler(int signum, siginfo_t *info, void *ucontext );

#endif
