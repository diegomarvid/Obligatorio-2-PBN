#include <stdlib.h>
#include <stdio.h>
#include "constantes.h"
#include "DynList.h"



DynList *dynList_crear(void) {

    DynList *dynlist;

    dynlist = (DynList *) malloc(sizeof(DynList));

    if(dynlist == NULL) {
        return NULL;
    } else {
        dynlist->head = NULL;
        dynlist->size = 0;
        return dynlist;
    }

}


Nodo *agregar_nodo(DynList *dynlist, int data){

    if(dynlist->size == 0) {

        Nodo *nuevo = (Nodo*)malloc(sizeof(Nodo));

        if (nuevo == NULL){
        
            return NULL;

        }else{
            //Agrego el nodo al final.
            nuevo->next = NULL;

            nuevo->data = data;

            dynlist->head = nuevo;
            //Adjudico al penultimo su valor de next.
        

            dynlist->size++;

            return nuevo;
        }
    }


    Nodo *actual = dynlist->head;
    Nodo *aux;
    //Busco el proximo espacio libre.
    do
    {
        aux = actual;

        actual = actual->next;

    } while (actual != NULL);

    //creo un nodo nuevo el la ultima posicion.

    Nodo *nuevo = (Nodo*)malloc(sizeof(Nodo));

    if (nuevo == NULL){
        
        return NULL;

    }else{
        //Agrego el nodo al final.
        nuevo->next = NULL;

        nuevo->data = data;

        //Adjudico al penultimo su valor de next.
        aux->next = nuevo;

        dynlist->size++;

        return nuevo;
    }

}


// void map_dynlist(void (*f)(Nodo *nodo)){
//     Nodo *current = dynlist->head;
// 	while (current != NULL) {
// 		f(current);
// 		current = current->next;
// 	}
// }

void print_dynlist(DynList *dynlist)
{
	Nodo *current = dynlist->head;

	while (current != NULL) {

		printf("%d->", current->data);
		current = current->next;
	}

    printf("\n");
}

int eliminar_nodo(DynList *dynlist, int dato){
    
    Nodo *head = dynlist->head;

    Nodo *actual = head;

    Nodo *aux;
//return
    
    while ( actual != NULL && actual->data != dato  )
    {

        aux = actual;

        actual = actual->next;

    } 
    
    
    
    if(actual == NULL){

        return FALLO;

    }else if (actual == head)
    {
        
        dynlist->head = actual->next;

        free(actual);

        return EXITO;


    }else if (actual->next == NULL)
    {

        aux->next = NULL;

        free(actual);

        return EXITO;

    }else
    {
        
        aux->next = actual->next;

        free(actual);

        return EXITO;

    }
    
    
}


// int main(int argc, char const *argv[])
// {

//     DynList *din = dynList_crear();
//     int dato;
   
//     while (1)
//     {
        
//         printf ("ingrese un numero prro: \n");
//         scanf("%d", &dato);
//         agregar_nodo(din,dato);
//         print_dynlist(din);

//         printf ("ingrese un numero prro: \n");
//         scanf("%d", &dato);
//         agregar_nodo(din,dato);
//         print_dynlist(din);

//         printf ("ingrese un numero prro a eliminar prro: \n");
//         scanf("%d", &dato);
//         eliminar_nodo(din,dato);
//         print_dynlist(din);
       
//     }
    

//     return 0;
// }







