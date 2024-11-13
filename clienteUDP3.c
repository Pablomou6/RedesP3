#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>

int main(int argc, char *argv[]) {
    if (argc != 5) {  
        printf("El número de parámetros no es el correcto.");
        exit(EXIT_FAILURE);
    }

    char archivoIN[20], archivoOUT[20];
    strncpy(archivoIN, argv[1], sizeof(archivoIN) - 1);
    archivoIN[sizeof(archivoIN) - 1] = '\0';

    FILE *archivo = fopen(argv[1], "r");
    if (archivo == NULL) { 
        perror("Error al abrir el archivo de entrada");
        exit(EXIT_FAILURE);
    }

    int puerto_propio = atoi(argv[2]);
    int puerto_dest = atoi(argv[4]);
    char *ip = argv[3];

    for (int i = 0; i < strlen(archivoIN); i++) {
        archivoOUT[i] = (archivoIN[i] != '.') ? toupper(archivoIN[i]) : archivoIN[i];
    }
    archivoOUT[strlen(archivoIN)] = '\0'; 

    FILE *salida = fopen(archivoOUT, "w");
    if (salida == NULL) { 
        perror("Error al crear el archivo de salida");
        fclose(archivo);
        exit(EXIT_FAILURE);
    }

    char linea[1024], buffer[1024];
    int socket_emisor, b, c;
    struct sockaddr_in dir_propia, dir_remota;
    socklen_t dir_len = sizeof(dir_remota);

    if ((socket_emisor = socket(AF_INET, SOCK_DGRAM, 0)) == 0) {
        perror("No se pudo crear el socket.\n");
        fclose(archivo);
        fclose(salida);
        exit(EXIT_FAILURE);
    }

    dir_propia.sin_family = AF_INET;
    dir_propia.sin_addr.s_addr = htonl(INADDR_ANY);
    dir_propia.sin_port = htons(puerto_propio);

    dir_remota.sin_family = AF_INET;
    dir_remota.sin_port = htons(puerto_dest);

    if (inet_pton(AF_INET, ip, &dir_remota.sin_addr) <= 0) {
        printf("Dirección no válida.\n");
        close(socket_emisor);
        fclose(archivo);
        fclose(salida);
        exit(EXIT_FAILURE);
    }

    if (bind(socket_emisor, (struct sockaddr *)&dir_propia, sizeof(dir_propia)) < 0) {
        perror("No se pudo asignar direccion.\n");
        close(socket_emisor);
        fclose(archivo);
        fclose(salida);
        exit(EXIT_FAILURE);
    }

    int j = 0;
    while(fgets(linea, sizeof(linea), archivo) != NULL) {
        printf("Linea %d: %s", ++j, linea);

        sleep(3);
        c = sendto(socket_emisor, linea, strlen(linea), 0, (struct sockaddr*)&dir_remota, dir_len);
        if (c < 0) {
            perror("Error al enviar datos");
            close(socket_emisor);
            fclose(archivo);
            fclose(salida);
            exit(EXIT_FAILURE);
        }

        usleep(50000);

        b = recvfrom(socket_emisor, buffer, sizeof(buffer) - 1, 0, (struct sockaddr*) &dir_remota, &dir_len);
        if (b < 0) {
            perror("No se pudo recibir los datos del servidor.\n");
            close(socket_emisor);
            fclose(archivo);
            fclose(salida);
            exit(EXIT_FAILURE);
        }
        buffer[b] = '\0'; 
        printf("MENSAJE DEL SERVIDOR EN MAY: %s\n", buffer);

        // Añadimos una nueva línea después de cada línea recibida
        fprintf(salida, "%s", buffer);
    }

    close(socket_emisor);
    fclose(archivo);
    fclose(salida);

    return 0;
}
