#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "constantes.h"

int main(int argc, char const *argv[])
{
    int i;

    for(i = 0; i < 4; i++) {
        printf("[%d] %d \n", getpid(), i);
        sleep(1);
    }

    return 0;
}
