#ifndef _L_H_
#define _L_H_

void intitiaze_monitor_fd_set(void);

void add_to_monitored_fd_set(int skt_fd);

void remove_from_monitored_fd_set(int skt_fd);

int get_max_fd(void);

void refresh_fd_set(fd_set *fd_set_ptr);

void cerrar_sockets(void);

void sigTermHandler(int signum, siginfo_t *info, void *ucontext);

void sigTermSet(void);

#endif