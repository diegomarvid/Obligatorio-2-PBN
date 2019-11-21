#ifndef CONSTANTES_H
#define CONSTANTES_H

enum { FALSE , TRUE };

#define CMD_SIZE 40
#define MSG_SIZE 100
#define PROCESS_MAX 100
#define SHM_ADDR "/tmp/shm_pbn"
#define PROJ_ID 'S'


#define MYERR(status, txt) error_at_line(status, errno, __FILE__, __LINE__ - 1, txt)

typedef struct {

    pid_t RID;

    int op;

    // int size;

    char data[CMD_SIZE]; 

} Mensaje;

typedef struct {

    pid_t RID;

    int estado;

    pid_t pid;

    char cmd[CMD_SIZE];


} Proceso;

#define SHM_SIZE sizeof(Proceso) * PROCESS_MAX

#define OFFSET 3 * sizeof(Proceso)


enum { INVALIDO = - 1 , CREAR, EJECUTANDO , SUSPENDIDO , ELIMINAR , TERMINADO};

enum { FALLO = - 1 , EXITO };

#endif