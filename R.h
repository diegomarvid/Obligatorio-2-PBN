#ifndef R_H
#define R_H

//Rp -> R prima

pid_t crear_Rp(void); // Asignar conexion a R' con consola

int cerrar_lista_Rp(void); //Eliminar todos los R' al cerrar el sistema

int eliminar_Rp(void); //Eliminar R' de la lista al cierre de conexion

void sigChildHandler(int signum, siginfo_t *info, void *ucontext );

#endif
