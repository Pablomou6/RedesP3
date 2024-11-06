#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>

int main(int argc, char *argv[]){

    //Comprobamos de que el numero de parametros es el correcto
    if (argc != 5) {  //argc debe ser 5: el nombre del programa, el archivo de entrada, el puerto propio, la IP y el puerto del servidor
        printf("El número de parámetros no es el correcto.");
        exit(EXIT_FAILURE);
    }

    char archivoIN[20];
    strncpy(archivoIN, argv[1], sizeof(archivoIN) - 1);
    archivoIN[sizeof(archivoIN)] = '\0';
    char archivoOUT[20];

    FILE *archivo = fopen(argv[1], "r"); //Se abre el archivo de entrada, que se pasa por parámetro
    if (archivo == NULL) { // Verificamos si hubo un error al abrir o crear el archivo
        printf("Error al crear el archivo");
        exit(EXIT_FAILURE);
    }

    int puerto_propio, puerto_dest; //Variables para guardar los puertos, tanto el propio como el de destino
    puerto_propio = atoi(argv[2]); //Guardamos en puerto_propio el puerto que se pasa por parametro al ejecutar el codigo
    puerto_dest = atoi(argv[4]); //Guardamos en puerto_dest el puerto que se pasa por parametro al ejecutar el codigo

    char *ip = argv[3]; //Se guarda en una variable la ip del emisor, que se pasa por parámetro
    
    int i;
    //Convertimos el nombre del archivo a mayúsculas
    for (i = 0; i < strlen(archivoIN); i++) {
        if (archivoIN[i] != '.') {
            archivoOUT[i] = toupper(archivoIN[i]);
        } else {
            archivoOUT[i] = archivoIN[i];
        }
    }
    // Añadir '\0' al final para evitar caracteres indeseados
    archivoOUT[i + 1] = '\0';
    //Creamos el archivo de salida y lo abre en modo escritura
    FILE *salida = fopen(archivoOUT, "w");
    if (salida == NULL) { //Verificamos si hubo un error al abrir o crear el archivo
        printf("Error al crear el archivo");
        exit(EXIT_FAILURE);
    }
    
    char linea[2048];//Variable en la que se ira guardando las líneas que se leen del archivo
    char buffer[2048]; //Buffer para guardar el mendaje que recibimos
    int b; //Variable para guardar el número de bytes que recibimos
    int c; //Variable para guardar el número de bytes que enviamos

    int socket_emisor; //Variable para el socket del emisor
    struct sockaddr_in dir_propia, dir_remota; //Variables que guardan una estructura en donde se guardan las direcciones y los puertos tanto propios como remotos
    socklen_t dir_len; //Variable para guardar el tamaño de las variables que guarda una estructura sockaddr_in
    
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

    dir_len = sizeof(dir_remota);
    
    int j = 0; //Variable para tener cuenta de el número de líneas que se leen
    //Bucle en el que se recorre todas las líneas del archivo de entrada y se va enviando una a una al servidor. En cada iteracion se envia una linea, se espera a que el servidor la devuelva en mayúsculas, y se añade al archivo de salida
    while(fgets(linea, sizeof(linea), archivo) != NULL){
        printf("Entre en el bucle\n");
        j = j +1; //Comenzamos contando en 1 y en cada iteracion sumamos 1;
        printf("Linea %d: %s", j, linea); //Imprimimos la linea de la iteracion actual

        c = sendto(socket_emisor, linea, strlen(linea), 0, (struct sockaddr*)&dir_remota, dir_len); //Enviamos la línea que acabamos de leer del archivo
        if (c < 0) {
            printf("Error al enviar datos");
            exit(EXIT_FAILURE);
        }

        usleep(50000); //Metemos un usleep equivalente a una espera de 50000 microsegundos para dar tiempo al servidor a recibir la linea actual y mandarla de vuelta en mayúsculas

        //Con recv obtenemos el mensaje del servidor (la linea en mayúsculas), y los bytes que ocupa, que guardamos en b. En la funcion indicamos el socket que usa el cliente, el buffer en el que se va a guardar el mensaje (buffer), el tamaño del buffer-1 ya que se reserva un hueco para el '\0', y por último las flags (no usamos en nuestro caso)
        if ((b = recvfrom(socket_emisor, buffer, sizeof(buffer)-1, 0, (struct sockaddr*) &dir_remota, &dir_len)) < 0){
            printf("No se pudo recibir los datos del servidor.\n");
            exit(EXIT_FAILURE);
        }
        buffer[b] = '\0'; //Se añade \0 para indicar el final de la cadena. Usamos la variable bytes para ponerla al final del buffer
        printf("MENSAJE DEL SERVIDOR EN MAY: %s\n", buffer); //Imprimimos el mensaje

        fputs(buffer, salida); //Metemos la linea transformada en mayúsculas en el archivo de salida
    }

    //Cerramos el socket del emisor con la funcion close()
    close(socket_emisor);
}