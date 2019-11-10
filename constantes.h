#ifndef CONSTANTES_H
#define CONSTANTES_H

typedef int PID;

enum { FALSE , TRUE };

#define CMD_SIZE 20
#define MSG_SIZE 100
#define PROCESS_MAX 10

#define MYERR(status, txt) error_at_line(status, errno, __FILE__, __LINE__ - 1, txt)

typedef struct {

    pid_t RID;

    int op;

    // int size;

    char data[MSG_SIZE]; 

} Mensaje;

typedef struct {

    pid_t RID;

    int estado;

    pid_t pid;

    char cmd[CMD_SIZE];


} Proceso;

enum { INVALIDO , EJECUTANDO , SUSPENDIDO , ELIMINAR , TERMINADO };

enum { FALLO = - 1 , EXITO };

#define INEXISTENTE 32512

#endif