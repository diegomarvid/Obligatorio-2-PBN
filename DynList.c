#include <stdlib.h>
#include <stdio.h>
#include "constantes.h"
#include "DynList.h"



//-----------------Crear lista dinamica------------------//

DynList *dynList_crear(void) {

    //Crea la lista vacia

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
//------------------------------------------------------//


//-----------------Agregar nodo lista-------------------//
Nodo *agregar_nodo(DynList *dynlist, int data, int fd){

    if(dynlist->size == 0) {

        Nodo *nuevo = (Nodo*)malloc(sizeof(Nodo));

        if (nuevo == NULL){

            return NULL;

        }else{
            //Agrego el nodo al final.
            nuevo->next = NULL;

            nuevo->data = data;

            nuevo->fd = fd;

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
        nuevo->fd = fd;

        //Adjudico al penultimo su valor de next.
        aux->next = nuevo;

        dynlist->size++;

        return nuevo;
    }

}
//------------------------------------------------------//



//-----------------Imprimir lista-----------------------//
void print_dynlist(DynList *dynlist)
{
	printf("\n");

    Nodo *current = dynlist->head;

	while (current != NULL) {

		printf("[%d-%d]->", current->data, current->fd);
		current = current->next;
	}

    printf("EL tamaño es: %d \n", dynlist->size);

    printf("\n");
}

//------------------------------------------------------//


//----------------Buscar nodo segun fd------------------//
Nodo *buscar_nodo_fd(DynList *dynlist, int fd) {

    Nodo *actual = dynlist->head;

    while ( actual != NULL && actual->fd != fd )
    {
        actual = actual->next;
    }

    return actual;
}
//------------------------------------------------------//


//----------------Buscar nodo segun data----------------//

Nodo *buscar_nodo_data(DynList *dynlist, int data) {

    Nodo *actual = dynlist->head;

    while ( actual != NULL && actual->data != data )
    {
        actual = actual->next;
    }

    return actual;
}

//------------------------------------------------------//


//--------------Eliminar nodo segun data----------------//

int eliminar_nodo(DynList *dynlist, int dato){

    Nodo *head = dynlist->head;

    Nodo *actual = head;

    Nodo *aux;

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

        dynlist->size--;

        free(actual);

        return EXITO;


    }else if (actual->next == NULL)
    {

        aux->next = NULL;

        dynlist->size--;

        free(actual);

        return EXITO;

    }else
    {

        aux->next = actual->next;

        dynlist->size--;

        free(actual);

        return EXITO;

    }

}

//------------------------------------------------------//


//---------------Eliminar nodo segun fd-----------------//
int eliminar_nodo_fd(DynList *dynlist, int fd){

    Nodo *head = dynlist->head;

    Nodo *actual = head;

    Nodo *aux;
//return

    while ( actual != NULL && actual->fd != fd  )
    {

        aux = actual;

        actual = actual->next;

    }



    if(actual == NULL){

        return FALLO;

    }else if (actual == head)
    {

        dynlist->head = actual->next;

        dynlist->size--;

        free(actual);



        return EXITO;


    }else if (actual->next == NULL)
    {

        aux->next = NULL;

        dynlist->size--;

        free(actual);

        return EXITO;

    }else
    {

        aux->next = actual->next;

        dynlist->size--;

        free(actual);

        return EXITO;

    }


}

