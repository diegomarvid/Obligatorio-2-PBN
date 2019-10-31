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


#endif