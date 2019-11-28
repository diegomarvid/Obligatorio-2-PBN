
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <error.h>
#include <errno.h>
#include <fcntl.h> // para el mkfifo
#include <sys/stat.h> // para el mkfifo
#include <sys/types.h>// para el mkfifo
#include "constantes.h"



int main(int argc, char const *argv[])
{
    
    char pipe_addr[100];

    char buffer[OUT_BUFFSIZE];

    strcpy(pipe_addr, argv[1]);

    int pipe_fd = open(pipe_addr, O_RDONLY);

    if(pipe_fd == FALLO) {
        MYERR(EXIT_FAILURE, "L no pudo acceder a la pipe");
    }

    int r = 4;

    do {
        //r = read(pipe_fd, buffer, OUT_BUFFSIZE);
        sleep(1);
    }  while (r != 0);

    
    return 0;
}
