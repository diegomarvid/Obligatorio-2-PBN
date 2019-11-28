# Obligatorio-2-PBN
Obligatorio de Progamacion de bajo nivel el cual funciona como un administrador de procesos en Linux Ubuntu

El proposito del obligatorio es un servidor que administra y ejecuta procesos de los clientes, el mismo debe:

* Crear
* Eliminar
* Suspender
* Reanudar
* Ver salida

De un proceso cualquiera dado su pid.

Asimismo, el usuario podra cerrar el sistema si el lo desea.

El bloque principal se llama MM y este se ejecuta con el comando ./MM dentro de la carpeta del obligatorio. Sin embargo,
es necesario compilar todos los procesos por lo cual existe un compilador el cual se ejecuta con el comando:
./compilador

El procedimiento usual para iniciar el sistema en una nueva computadora seria el siguiente:
> ./compilador

> ./MM

 Para conectarse desde una computadora al servidor es necesario hacerlo con el programa consola
 esto se haria de la siguiente forma:
 > gcc -Wall consola.c -o consola
 
 > ./consola
 
 Sin embargo ejecutando ./compilador tambien compila consola, por lo cual se podria hacer lo siguiente tambien:
 > ./compilador
 
 > ./consola
 
 Esto es practico por si se quiere usar la consola y el servidor en el mismo dispositivo. Asi todo se compila con un
 unico comando.
