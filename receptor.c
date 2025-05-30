#include<stdio.h>
#include<stdlib.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<string.h>
#include<unistd.h>

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

    /**
     * 1.c)
     * modificamos la función recvfrom actual y ponemos que reciba 2 bytes. Dejmos que imprima el mensaje que recibió y volveremos a llamar a la función.
     * Tras las líneas de impresión del mensaje, pegamos otra llamada a recvfrom:
     * tamMsg = recvfrom(Socket, string, 1023, 0, (struct sockaddr*) &direccionRemota, &tamanho);
        if(tamMsg < 0) {
            perror("Error al recibir el mensaje.\n");
            exit(EXIT_FAILURE);
        }
        A continuación, volvemos a imprimir el mensaje:
        printf("Se ha recibido un mensaje por el puerto %d. Enviado por %s.\n", ntohs(direccionRemota.sin_port), IP);
        printf("Se han recibido %zd bytes.\n", tamMsg);
        printf("El mensaje es: %s\n", string);

        El resultado será que el receptor recibirá 2 bytes de mensaje y se quedará a la espera en la segunda llamada a recvfrom. Si volvemos a ejecutar el 
        emisor, recibirá el mensaje entero, pero si no lo ejecutamos, no sucederá. Esto se debe a que UDP no es un protocolo que mantenga el estado del flujo
        de datos, entonces los bytes sobrantes (que no caben e el buffer) se perderán.
     */

    tamanho = sizeof(direccionRemota);
    ssize_t tamMsg = recvfrom(Socket, string, 1023, 0, (struct sockaddr*) &direccionRemota, &tamanho);
    if(tamMsg < 0) {
        perror("Error al recibir el mensaje.\n");
        exit(EXIT_FAILURE);
    }

    string[tamMsg] = '\0';

    char IP[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(direccionRemota.sin_addr), IP, INET_ADDRSTRLEN);
    printf("Se ha recibido un mensaje por el puerto %d. Enviado por %s.\n", ntohs(direccionRemota.sin_port), IP);
    printf("Se han recibido %zd bytes.\n", tamMsg);
    printf("El mensaje es: %s\n", string);
}