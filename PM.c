
#include "funcionesPM.c"

//*******Funciones de signals********//





int main(int argc, char const *argv[])
{


    //Seteo interrupciones para Child
    sigChildSet();
    sigTermSet();

    sem = sem_open(SEM_ADDR, O_CREAT, 0666, 1);

    int mm_socket;
    mm_socket = sock_connect_un(SOCKET_NAME);

    if( mm_socket < 0 ){
        MYERR(EXIT_FAILURE, "Error, no se pudo aceptar conexion. \n");
    }

    printf("[PM] Conexion exitosa con MM\n");

    lista_proceso = obtener_shm(OFFSET);

    ejecutar_procesos(mm_socket);

    eliminar_procesos();

    sem_close(sem);
    shmdt((void*)obtener_shm(0));
    


    return 0;
}
