#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h> // Socket addresses
#include <arpa/inet.h> // Socket addresses
#include "sock.h"

#define MAX_CLIENTS 1
#define PORT 3045
#define SERVERHOST "127.0.0.1"
#define BUFFSIZE 256



int sock_listen(uint16_t port){

    int sock;
    struct sockaddr_in name;

    /* Create socket */
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if(sock < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    /* Give the socket a name */
    name.sin_family = AF_INET;
    name.sin_port = htons(port);
    name.sin_addr.s_addr = htonl(INADDR_ANY);

    /* Do the binding */
    if(bind(sock, (struct sockaddr *) &name, sizeof(name)) < 0){
        perror("bind");
        exit(EXIT_FAILURE);
    }

    printf("bind() successfull \n");

    if(listen(sock, MAX_CLIENTS) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }


    return sock;

}



int sock_open(int sock) {

    /* Accept connection */
    struct sockaddr addr;
    socklen_t length = sizeof(struct sockaddr);

    printf("\n");

    printf("Waiting on accept() \n");

    printf("\n");

    int rdsocket = accept(sock, &addr, &length);
    if(rdsocket < 0) {
        perror("No pude conectar el servidor");
    }

    printf("Connection accepted from client\n");

    printf("\n");

    return rdsocket;

}

int sock_connect(char *address, uint16_t port) {
    int sock;
    struct sockaddr_in servername;

    /* Create the socket */
    sock = socket (PF_INET, SOCK_STREAM, 0);
    if(sock < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    /* Give the socket a name */
    servername.sin_family = AF_INET;
    servername.sin_port = htons(port);
    if(address == NULL) {
        servername.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    } else {
        if(inet_aton(address, &servername.sin_addr) == 0) {
            perror("addres name");
            exit(EXIT_FAILURE);
        }
    }

    /* Do connect */
    if(connect(sock, (struct sockaddr *) &servername, sizeof(servername)) == -1) {
        perror("connect (client)");
        exit(EXIT_FAILURE);
    }

    printf("Connected to the server \n");

    return sock;


}