#ifndef MM_H
#define MM_H

void intitiaze_monitor_fd_set(void);

void add_to_monitored_fd_set(int skt_fd);

void remove_from_monitored_fd_set(int skt_fd);

int get_max_fd(void);

void refresh_fd_set(fd_set *fd_set_ptr);

int request_process_space(void);

int obtener_estado(pid_t pid);

int agregar_proceso(char cmd[], int RID);

int cambiar_estado_proceso(pid_t pid, int estado);

void formatear_estado(Mensaje *mensaje, pid_t pid, int estado);

void obtener_lista(Mensaje *mensaje);

void ejecutar_operacion(Mensaje *mensaje, int socket_actual);

void iniciar_sistema(int connection_socket);

void cerrar_proceso(pid_t pid, int tiempo);

void eliminar_sistema(void);

#endif
