#ifndef RP_H
#define RP_H

void conv_to_struct(Mensaje *mensaje, char buffer[]);

void refresh_fd_set(fd_set *fd_set_ptr);

int get_max_fd();

void sigTermHandler(int signum, siginfo_t *info, void *ucontext);

void sigTermSet(void); 

#endif