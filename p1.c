#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "constantes.h"

int main(int argc, char const *argv[])
{
    int i;

    sleep(10);

    for(i = 0; i < 50; i++) {
        printf("[%d] %d ~ \n", getpid(), i);
        //sleep(1);
    }

    //execl("dasdasdd", "sadsd", NULL);

    return 0;
}
