#ifndef CONSTANTES_H
#define CONSTANTES_H

enum { FALSE , TRUE };

#define CMD_SIZE 40
#define MSG_SIZE 100
#define PROCESS_MAX 100
#define SHM_ADDR "/tmp/shm_pbn"
#define PROJ_ID 'S'
#define PID_SIZE 9
#define DATA_SIZE PROCESS_MAX * (PID_SIZE + 1 + CMD_SIZE + 1)
#define RESPUESTA_BUFFSIZE PID_SIZE + 1 + DATA_SIZE
#define ENTRADA_BUFFSIZE PID_SIZE + 1 + CMD_SIZE


#define MYERR(status, txt) error_at_line(status, errno, __FILE__, __LINE__ - 1, txt)

typedef struct {

    pid_t RID;

    int op;

    char data[DATA_SIZE];

    int id; 

} Mensaje;

typedef struct {

    pid_t RID;

    int estado;

    pid_t pid;

    char cmd[CMD_SIZE];


} Proceso;

#define SHM_SIZE sizeof(Proceso) * PROCESS_MAX

#define OFFSET 3

//Lista de operaciones a ejecutar
enum { CREACION = 1, ELIMINACION, SUSPENCION, RENAUDAR, ESTADO, LISTA, CERRAR_CONSOLA, CERRAR_SISTEMA};

//Posibles estados de un proceso
enum { INVALIDO = - 1 , CREAR, EJECUTANDO , SUSPENDIDO , ELIMINAR , TERMINADO};

enum { FALLO = - 1 , EXITO };

enum { RP , PM , MM };

enum { SINCRONICO, ASINCRONICO };



#endif