#ifndef CONSTANTES_H
#define CONSTANTES_H

enum { FALSE , TRUE };

#define CMD_SIZE 40
#define MSG_SIZE 100
#define PROCESS_MAX 100
#define SHM_ADDR "/tmp/shm_pbn"
#define PROJ_ID 'S'
#define PIPE_ADDR "/tmp/pipe_"
#define L_ADDR "/tmp/listener_"
#define SEM_ADDR "sem_pbn"
#define SOCKET_NAME "/tmp/PBN"


#define MAX_CLIENTS 20
#define PORT 3045
#define SERVERHOST "127.0.0.1"

//pid maximo tiene 9 digitos, es el size asociado a su str equivalente
#define PID_SIZE 9

//Caso maximo de data es cuando se envia una lista:
/*    cantidad de procesos * pid-cmd'27'    */
#define DATA_SIZE PROCESS_MAX * (PID_SIZE + 1 + CMD_SIZE + 1)

//Respuesta es del tipo
/*          sincrono/asicrono-data          */
#define RESPUESTA_BUFFSIZE PID_SIZE + 1 + DATA_SIZE

//Entrada es del tipo
/*               op-cmd                     */
#define ENTRADA_BUFFSIZE PID_SIZE + 1 + CMD_SIZE

//Buffer para leer y enviar salida estandar de un proceso
#define OUT_BUFFSIZE 500


#define MYERR(status, txt) error_at_line(status, errno, __FILE__, __LINE__ - 1, txt)

typedef struct {

    pid_t RID;

    int op;

    char data[DATA_SIZE];

    int id;

} Mensaje;

typedef struct {

    pid_t RID;

    pid_t LID;

    int estado;

    pid_t pid;

    char cmd[CMD_SIZE];

} Proceso;

#define SYSTEM_PROCESS_SIZE 3

#define TOTAL_PROCESS (SYSTEM_PROCESS_SIZE + PROCESS_MAX)

#define SHM_SIZE  (sizeof(Proceso) * TOTAL_PROCESS)



//Offser para SHM
#define OFFSET 3

//Lista de operaciones a ejecutar
enum { CREACION = 1, ELIMINACION, SUSPENCION, RENAUDAR, ESTADO, LISTA, CERRAR_CONSOLA, CERRAR_SISTEMA, LEER_SALIDA};

//Posibles estados de un proceso
enum { INVALIDO = - 1 , CREAR, EJECUTANDO , SUSPENDIDO , ELIMINAR , TERMINADO};

enum { FALLO = - 1 , EXITO };

//Identificador de comunicacion
enum { RP , PM , MM };

//Tipo de comunicacion
enum { SINCRONICO, ASINCRONICO };

//Errores de socket
enum { ERROR_CONNECTION = -1 , END_OF_CONNECTION };





#endif
