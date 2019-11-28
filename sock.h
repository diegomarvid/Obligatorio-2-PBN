#ifndef _SOCK_H_
#define _SOCK_H_

int sock_listen_un (char *socketaddr);

int sock_listen_in(uint16_t port);

int sock_open_un(int connection_socket);

int sock_open_in(int sock);

int sock_connect_un(char *sockadrr);

int sock_connect_in(char *address, uint16_t port);

#endif
