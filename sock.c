#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h> // Socket addresses
#include <arpa/inet.h> // Socket addresses
#include <sys/un.h>
#include <errno.h>
#include "constantes.h"
#include "sock.h"


#define MAX_CLIENTS 20
#define PORT 3045
#define SERVERHOST "127.0.0.1"
#define SOCKET_NAME "/tmp/PBN"


int sock_listen_un (){

    int connection_socket;

    struct sockaddr_un name;

    /* Create socket */

    connection_socket = socket(AF_UNIX, SOCK_STREAM, 0);

    if(connection_socket < 0) {

        perror("socket");
        exit(EXIT_FAILURE);

    }

    printf("Master socket filed descriptor created \n");

    memset(&name, 0, sizeof(struct sockaddr_un));

    name.sun_family = AF_UNIX;

    strncpy(name.sun_path, SOCKET_NAME, sizeof(name.sun_path) - 1);

     /* Do the binding */
    if(bind(connection_socket, (const struct sockaddr *) &name, sizeof(name)) < 0){

        perror("bind");

        exit(EXIT_FAILURE);

    }

    printf("bind() successfull \n");

    if(listen(connection_socket, MAX_CLIENTS) < 0) {

        perror("listen");

        exit(EXIT_FAILURE);
    }


    return connection_socket;

}


int sock_listen_in(uint16_t port){

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

int sock_open_un(int connection_socket) {

int rdsocket;

    do{

        printf("\n");

        printf("Waiting on accept() \n");

        printf("\n");

        rdsocket = accept(connection_socket, NULL, NULL);
        

    }while(errno == EINTR && rdsocket < 0);


    if(rdsocket < 0) {
        perror("No pude conectar el servidor");
    }

    printf("Connection accepted from client\n");

    printf("\n");

    return rdsocket;

}

int sock_open_in(int sock) {

    /* Accept connection */
    struct sockaddr addr;
    socklen_t length = sizeof(struct sockaddr);

    int rdsocket;

    do{

        printf("\n");

        printf("Waiting on accept() \n");

        printf("\n");

        rdsocket = accept(sock, &addr, &length);
        

    }while(errno == EINTR && rdsocket < 0);


    if(rdsocket < 0) {
        perror("No pude conectar el servidor");
    }

    printf("Connection accepted from client\n");

    printf("\n");

    return rdsocket;
}


int sock_connect_un() {

    int sock;

        struct sockaddr_un addr;

        /* Create the socket */
        sock = socket(AF_UNIX, SOCK_STREAM, 0);

        if(sock < 0) {

            perror("socket");
            exit(EXIT_FAILURE);

        }

        memset(&addr, 0, sizeof(struct sockaddr_un));

        /* Connect socket to socket address */

        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, SOCKET_NAME, sizeof(addr.sun_path) - 1);

           /* Do connect */
        if(connect(sock, (const struct sockaddr *) &addr, sizeof(struct sockaddr_un)) == -1) {

            perror("connect (client)");
            exit(EXIT_FAILURE);

        }

    printf("Connected to the server \n");

    return sock;

}


int sock_connect_in(char *address, uint16_t port) {

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