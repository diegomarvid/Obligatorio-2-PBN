#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include "sock.c"
#include "constantes.h"
#include "MM.h"


void print(){
    printf("Hello world \n");
}

void ejemplo(void (*f)()){
    f();
}







int main(int argc, char const *argv[])
{

    for (size_t i = 0; i < 5; i++)
    {
        /* code */
        int wrsocket = sock_connect("127.0.0.1", 2000); 

        
        sleep(1);
    }
    
       
    //ejemplo(print);
    
    return 0;
}
