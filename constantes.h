#ifndef CONSTANTES_H
#define CONSTANTES_H

typedef int PID;

enum { FALSE , TRUE };

#define CMD_SIZE 20
#define MSG_SIZE 100
#define PROCESS_MAX 10

typedef struct {

    PID RID;

    int op;

    int size;

    char data[MSG_SIZE]; 

} Mensaje;

typedef struct {

    PID RID;

    int estado;

    PID pid;

    char cmd[CMD_SIZE];


} Proceso;

enum { INVALIDO , EJECUTANDO , SUSPENDIDO , ELIMINAR , TERMINADO };

enum { FALLO = - 1 , EXITO };

#define INEXISTENTE 32512

#endif