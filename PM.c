#include "funcionesPM.c"



int main(int argc, char const *argv[])
{


    //---------Iniciar interrumpciones-----------//
    sigChildSet();
    sigTermSet();

    //-------------Iniciar semaforo--------------//
    sem = sem_open(SEM_ADDR, O_CREAT, 0666, 1);


    //----------Iniciar socket con MM------------//
    int mm_socket;
    mm_socket = sock_connect_un(SOCKET_NAME);

    if( mm_socket < 0 ){
        MYERR(EXIT_FAILURE, "Error, no se pudo aceptar conexion. \n");
    }

    printf("[PM] Conexion exitosa con MM\n");


    //---------Obtener lista de procesos-------//
    lista_proceso = obtener_shm(OFFSET);
    
    
    //---------Loop principal de ejecucion-----------//
    ejecutar_procesos(mm_socket);


    //-------------Eliminar procesos-----------------//
    eliminar_procesos();

    //-------------Cerrar semaforo y shm-------------//
    sem_close(sem);
    //Se le pasa donde empieza la shm para eliminarla
    shmdt((void*)obtener_shm(0));
    
    return 0;
}
