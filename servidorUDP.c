#include<stdio.h>
#include<stdlib.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<string.h>
#include<unistd.h>
#include<ctype.h>

//Función que convierte un str dado, en el mismo str, pero en mayúsculas
void convertToUpperCase(char *str) {
    //Bucle que, mientras la i-ésima posición del str es diferente del caracter de fin de línea, converte la posición en mayúscula
    for(int i = 0; str[i] != '\0'; i++) {
        str[i] = toupper(str[i]);
    }
}

int main(int argc, char* argv[]) {
    int Socket;
    struct sockaddr_in direccionPropia, direccionRemota;
    socklen_t tamanho;
    char string[1024];

    //Creamos el socket del receptor
    /**
     * @param AF_INET: Especificamos que usaremos IPv4
     * @param SOCK_STREAM: Indicamos el tipo de la conexión. Con "SOCK_DGRAM" nos referimos a un socket sin conexión
     * @param 0: Indicamos el protocolo que queremos usar. Al introducir un "0" usaremos el establecido por defecto
     */
    if((Socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Error al crear el Socket.\n");
        exit(EXIT_FAILURE);
    }

    //Inicializamos la primera estructura. Esta, que almacenará los datos propios, se inicializa con los siguientes valores: "AF_INET" para indicar que 
    //usaremos una IPv4. El puerto será el introducido por comandos, al que debemos convetir el formato de almacenamiento de bytes de host a network. Por
    //último, la dirección la indicaremos con un "INADDR_ANY" pasado por la función htonl, para así indicar que nos interesa aceptar a cualquier direccion IP.
    direccionPropia.sin_family = AF_INET;
    direccionPropia.sin_port = htons(atoi(argv[1]));
    direccionPropia.sin_addr.s_addr = htonl(INADDR_ANY);

    //Inicializamos la segunda estructura, pero solamente con el campo de "sin_family", para indicar el uso de la IPv4. El resto de campos serán sobreescritos
    //con la IP y el puerto del emisor.
    direccionRemota.sin_family = AF_INET;

    //Con la función bind, le asignamos dirección y puerto al socket. Serán los datos locales.
    /**
     * @param Socket: Socket la que queremos asignarle dirección y puerto.
     * @param direccionPropia: Debemos pasar la dirección de memoria de la estructura donde tenemos la diercción a asignar al socket (casteandola como struct sockaddr*)
     * @param sockaddr_in: Le pasamos, con sizeof, el tamaño  de la estructura de tipo sockaddr_in
     */
    if((bind(Socket, (struct sockaddr*) &direccionPropia, sizeof(struct sockaddr_in))) < 0) {
        perror("Error en la asignación de dirección y puerto.\n");
        exit(EXIT_FAILURE);
    }

    tamanho = sizeof(direccionRemota);
    ssize_t tamMsg = 1;
    while((tamMsg = recvfrom(Socket, string, 1023, 0, (struct sockaddr*) &direccionRemota, &tamanho)) > 0) {
        if(tamMsg < 0) {
            perror("Error al recibir el mensaje.\n");
            exit(EXIT_FAILURE);
        }

        string[tamMsg] = '\0';

        convertToUpperCase(string);

        socklen_t dir_len = sizeof(direccionRemota);
        //Enviamos el mensaje al receptor con la funcion sendto() (usa UDP). En la funcion se pasas por parametro el socket del emisor, el mensaje que se quiere enviar el receptor, el tamaño del mensaje, el 0 es porque hacemos uso de flags, La estructura con la dirección del socket destino (dir_remota) y su tamaño sacado con sizeof()
        int b = sendto(Socket, string, strlen(string), 0, (struct sockaddr*)&direccionRemota, dir_len);
        if (b < 0) {
            printf("Error al enviar datos");
            exit(EXIT_FAILURE);
        }
    }
    
}