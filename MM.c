#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
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
    ejemplo(print);
    
    return 0;
}
