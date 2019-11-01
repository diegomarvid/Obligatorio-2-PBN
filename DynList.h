#ifndef DYNLIST_H
#define DYNLIST_H

typedef struct Nodo {

    int data;

    struct Nodo *next;


} Nodo;

typedef struct DynList {

    Nodo *head;

    int size;

} DynList;

DynList *dynList_crear(void);

Nodo *agregar_nodo(DynList *dynlist, int data);

void print_dynlist(DynList *dynlist);

int eliminar_nodo(DynList *dynlist, int dato);



#endif