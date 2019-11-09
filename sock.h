#ifndef _SOCK_H_
#define _SOCK_H_

#include <sys/socket.h>
#include <netinet/in.h> // Socket addresses

int sock_listen (uint16_t port);

int sock_open (int sock);

int sock_connect (char* address, uint16_t port);

#endif
