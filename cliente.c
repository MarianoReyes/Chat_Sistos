#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Uso: %s [IP] [puerto]\n", argv[0]);
        exit(1);
    }

    int sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        printf("Error al crear el socket\n");
        exit(1);
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
    {
        printf("Error al conectar al servidor\n");
        exit(1);
    }

    char message[BUF_SIZE];
    printf("Escriba un mensaje para enviar al servidor: ");
    fgets(message, BUF_SIZE, stdin);

    int len = strlen(message);
    message[len - 1] = '\0'; // Elimina el salto de línea al final del mensaje

    char buffer[BUF_SIZE];
    memset(buffer, 0, BUF_SIZE);
    snprintf(buffer, BUF_SIZE, "5,%d,%s,", len, message);

    write(sock, buffer, strlen(buffer));

    memset(buffer, 0, BUF_SIZE);
    int read_len = read(sock, buffer, BUF_SIZE - 1);

    if (read_len == -1)
    {
        printf("Error al recibir la respuesta del servidor\n");
        exit(1);
    }

    printf("Respuesta del servidor: %s\n", buffer);

    close(sock);
    return 0;
}
