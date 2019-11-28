#ifndef DYNLIST_H
#define DYNLIST_H

typedef struct Nodo {

    int data;

    int fd;

    struct Nodo *next;


} Nodo;

typedef struct DynList {

    Nodo *head;

    int size;

} DynList;

DynList *dynList_crear(void);

Nodo *agregar_nodo(DynList *dynlist, int data, int fd);

void print_dynlist(DynList *dynlist);

Nodo *buscar_nodo_fd(DynList *dynlist, int fd);

Nodo *buscar_nodo_data(DynList *dynlist, int data);

int eliminar_nodo(DynList *dynlist, int dato);

int eliminar_nodo_fd(DynList *dynlist, int fd);



#endif