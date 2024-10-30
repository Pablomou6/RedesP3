#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>

int main(int argc, char *argv[]){


    int socket_emisor; //Variable para el socket del emisor
    struct sockaddr_in dir_propia, dir_remota; //Variables que guardan una estructura en donde se guardan las direcciones y los puertos tanto propios como remotos
    socklen_t dir_len; //Variable para guardar el tamaño de las variables que guarda una estructura sockaddr_in

    char *mensaje = "Ahi va Iago a la carrera con Akapo."; //Variable en la que se guarda el mensaje que se quiere enviar

    int puerto_propio, puerto_dest; //Variables para guardar los puertos, tanto el propio como el de destino
    puerto_propio = atoi(argv[1]); //Guardamos en puerto_propio el puerto que se pasa por parametro al ejecutar el codigo
    puerto_dest = atoi(argv[3]); //Guardamos en puerto_dest el puerto que se pasa por parametro al ejecutar el codigo

    char *ip = argv[2]; //Se guarda en una variable la ip del emisor, que se pasa por parámetro

    //Creación del socket del emisor, indicando que es de tipo IPv4 con AF_INET y que está orientado a la conexión con SOCK_STREAM
    if ((socket_emisor = socket(AF_INET, SOCK_DGRAM, 0)) == 0) {
        perror("No se pudo crear el socket.\n");
        exit(EXIT_FAILURE);
    }

    //Configuracion de los campos de la direccion propia 
    dir_propia.sin_family = AF_INET; //Indica que la ip va a ser de tipo IPv4
    dir_propia.sin_addr.s_addr = htonl(INADDR_ANY); //Variable que contiene la dirección IP del socket en formato binario. Lo pasamos a big-endian con htonl
    dir_propia.sin_port = htons(puerto_propio); //Variable que contiene el puerto en el que el socket escuchara conexiones. Lo pasamos a big-endian con htons

    //Configuracion de los campos de la direccion remota 
    dir_remota.sin_family = AF_INET; //Indica que la ip va a ser de tipo IPv4
    dir_remota.sin_port = htons(puerto_dest); //Variable que contiene el puerto en el que el socket escuchara conexiones. Lo pasamos a big-endian con htons
    
    //Convertimos la dirección IP a formato binario. Indicamos que es de tipo IPv4 con AF_INET, pasamos la variable que tiene la ip y la que contendra la ip en formato binario (address.sin_addr)
    if (inet_pton(AF_INET, ip, &dir_remota.sin_addr) <= 0) {
        printf("Dirección no válida.");
        exit(EXIT_FAILURE);
    }

    //Se asigna la direccion, pasandole el socket creado, la direccion y el puerto al que asignar el socket (gaurdado en dir_propia) y el numero de bytes que debe leer bind()
    if (bind(socket_emisor, (struct sockaddr *)&dir_propia, sizeof(dir_propia)) < 0) {
        perror("No se pudo asignar direccion.\n");
        exit(EXIT_FAILURE);
    }

    //Enviamos el mensaje al receptor con la funcion sendto() (usa UDP). En la funcion se pasas por parametro el socket del emisor, el mensaje que se quiere enviar el receptor, el tamaño del mensaje, el 0 es porque hacemos uso de flags, La estructura con la dirección del socket destino (dir_remota) y su tamaño sacado con sizeof()
    int b = sendto(socket_emisor, mensaje, strlen(mensaje), 0, (struct sockaddr*)&dir_remota, sizeof(dir_remota));
    if (b < 0) {
        printf("Error al enviar datos");
        exit(EXIT_FAILURE);
    }

    //Mostramos por pantalla un mensaje de confirmacion de el envio y el numero de bytes que se mandan al receptor
    printf("Mensaje enviado.\n");
    printf("Número de bytes enviados: %d\n", b);

    //Cerramos el socket del emisor con la funcion close()
    close(socket_emisor);
}