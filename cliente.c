#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "chat-2.pb-c.h"

#define BUFFER_SIZE 1024
#define PORT 8082

int sockfd = 0;

void print_menu()
{
    printf("Seleccione una opción:\n");
    printf("1. Crear nuevo usuario\n");
    printf("2. Ver usuarios conectados\n");
    printf("3. Cambiar estado\n");
    printf("4. Enviar mensaje\n");
    printf("5. Salir\n");
}

void create_user()
{
    ChatSistOS__NewUser createuser = CHAT_SIST_OS__NEW_USER__INIT;
    ChatSistOS__UserOption user_option = CHAT_SIST_OS__USER_OPTION__INIT;
    char buffer[BUFFER_SIZE];
    int bytes_sent, bytes_received;

    // Leemos el nombre de usuario FUNCIONAL LISTO
    printf("Introduzca un nombre de usuario: ");
    char username[BUFFER_SIZE];
    fgets(username, BUFFER_SIZE, stdin);
    username[strlen(username) - 1] = '\0';
    createuser.username = username;

    // Creamos el mensaje de opción de usuario
    user_option.op = 1;
    user_option.createuser = &createuser;
    
    // Serializamos el mensaje de opción de usuario
    size_t option_size = chat_sist_os__user_option__get_packed_size(&user_option);
    uint8_t option_buffer[option_size];
    chat_sist_os__user_option__pack(&user_option, option_buffer);

    // Enviamos el mensaje de opción de usuario al servidor
    bytes_sent = write(sockfd, option_buffer, option_size);
    if (bytes_sent < 0)
    {
        perror("Error al enviar el mensaje al servidor");
        return;
    }

    printf("\nllegamos hasta aca3");

    // Esperamos la respuesta del servidor
    bytes_received = read(sockfd, buffer, BUFFER_SIZE);
    if (bytes_received < 0)
    {
        perror("Error al recibir la respuesta del servidor");
        return;
    }

    printf("\nllegamos hasta aca4");

    // Decodificamos la respuesta
    ChatSistOS__Answer *response_message = chat_sist_os__answer__unpack(NULL, bytes_received, buffer);
    if (response_message == NULL)
    {
        perror("Error al decodificar la respuesta del servidor");
        return;
    }

    printf("\nllegamos hasta aca");

    // Imprimimos la respuesta del servidor
    printf("%s\n", response_message->response_message);

    chat_sist_os__answer__free_unpacked(response_message, NULL);
}

void view_users()
{
    ChatSistOS__UserOption user_option = CHAT_SIST_OS__USER_OPTION__INIT;
    char buffer[BUFFER_SIZE];
    int bytes_sent, bytes_received;

    // Creamos el mensaje de opción de usuario
    user_option.op = 2;
    int user_option_size = chat_sist_os__user_option__get_packed_size(&user_option);
    chat_sist_os__user_option__pack(&user_option, buffer);

    // Enviamos el mensaje al servidor
    bytes_sent = send(sockfd, buffer, user_option_size, 0);
    if (bytes_sent < 0)
    {
        perror("Error al enviar el mensaje al servidor");
        return;
    }

    // Esperamos la respuesta del servidor
    bytes_received = recv(sockfd, buffer, BUFFER_SIZE, 0);
    if (bytes_received < 0)
    {
        perror("Error al recibir la respuesta del servidor");
        return;
    }

    // Decodificamos la respuesta
    ChatSistOS__Answer *response_message = chat_sist_os__answer__unpack(NULL, bytes_received, buffer);
    if (response_message == NULL)
    {
        perror("Error al decodificar la respuesta del servidor");
        return;
    }

    // Imprimimos la respuesta del servidor
    printf("%s\n", response_message->response_message);

    chat_sist_os__answer__free_unpacked(response_message, NULL);
}

void change_status()
{
    ChatSistOS__Status status = CHAT_SIST_OS__STATUS__INIT;
    ChatSistOS__UserOption user_option = CHAT_SIST_OS__USER_OPTION__INIT;
    char buffer[BUFFER_SIZE];
    int bytes_sent, bytes_received;
    // Leemos el nuevo estado
    printf("Seleccione un estado (0 = desconectado, 1 = conectado, 2 = ocupado): ");
    scanf("%d", &status.user_state);
    getchar();

    // Creamos el mensaje de opción de usuario
    user_option.op = 3;
    user_option.status = &status;
    int user_option_size = chat_sist_os__user_option__get_packed_size(&user_option);
    chat_sist_os__user_option__pack(&user_option, buffer);

    // Enviamos el mensaje al servidor
    bytes_sent = send(sockfd, buffer, user_option_size, 0);
    if (bytes_sent < 0)
    {
        perror("Error al enviar el mensaje al servidor");
        return;
    }

    // Esperamos la respuesta del servidor
    bytes_received = recv(sockfd, buffer, BUFFER_SIZE, 0);
    if (bytes_received < 0)
    {
        perror("Error al recibir la respuesta del servidor");
        return;
    }

    // Decodificamos la respuesta
    ChatSistOS__Answer *response_message = chat_sist_os__answer__unpack(NULL, bytes_received, buffer);
    if (response_message == NULL)
    {
        perror("Error al decodificar la respuesta del servidor");
        return;
    }

    // Imprimimos la respuesta del servidor
    printf("%s\n", response_message->response_message);

    chat_sist_os__answer__free_unpacked(response_message, NULL);
}

void send_message()
{
    ChatSistOS__Message message = CHAT_SIST_OS__MESSAGE__INIT;
    ChatSistOS__UserOption user_option = CHAT_SIST_OS__USER_OPTION__INIT;
    char buffer[BUFFER_SIZE];
    int bytes_sent, bytes_received;

    // Leemos el destinatario y el mensaje
    printf("Introduzca el nombre del destinatario: ");
    fgets(message.message_destination, BUFFER_SIZE, stdin);
    message.message_destination[strlen(message.message_destination) - 1] = '\0';

    // Leemos el mensaje
    printf("Introduzca el mensaje: ");
    fgets(message.message_content, BUFFER_SIZE, stdin);
    message.message_content[strlen(message.message_content) - 1] = '\0';

    // Creamos el mensaje de opción de usuario
    user_option.op = 4;
    user_option.message = &message;
    int user_option_size = chat_sist_os__user_option__get_packed_size(&user_option);
    chat_sist_os__user_option__pack(&user_option, buffer);

    // Enviamos el mensaje al servidor
    bytes_sent = send(sockfd, buffer, user_option_size, 0);
    if (bytes_sent < 0)
    {
        perror("Error al enviar el mensaje al servidor");
        return;
    }

    // Esperamos la respuesta del servidor
    bytes_received = recv(sockfd, buffer, BUFFER_SIZE, 0);
    if (bytes_received < 0)
    {
        perror("Error al recibir la respuesta del servidor");
        return;
    }

    // Decodificamos la respuesta
    ChatSistOS__Answer *response_message = chat_sist_os__answer__unpack(NULL, bytes_received, buffer);
    if (response_message == NULL)
    {
        perror("Error al decodificar la respuesta del servidor");
        return;
    }

    // Imprimimos la respuesta del servidor
    printf("%s\n", response_message->response_message);

    chat_sist_os__answer__free_unpacked(response_message, NULL);
}

int main(int argc, char const *argv[])
{
    struct sockaddr_in serv_addr;
    // Creamos el socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("Error al crear el socket");
        return 1;
    }
    printf("\nSocket numero: %d",sockfd);

    // Configuramos la dirección del servidor
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
    {
        perror("Error al asignar la dirección del servidor");
        return 1;
    }

    // Conectamos con el servidor
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Error al conectarse con el servidor");
        return 1;
    }

    while (1)
    {
        // Imprimimos el menú
        print_menu();

        int option;
        printf("\nOpción seleccionada: ");
        scanf("%d", &option);
        getchar();

        switch (option)
        {
        case 1:
            create_user();
            break;
        case 2:
            view_users();
            break;
        case 3:
            change_status();
            break;
        case 4:
            send_message();
            break;
        case 5:
            printf("Saliendo...\n");
            close(sockfd);
            exit(0);
        default:
            printf("Opción no válida\n");
            break;
        }
    }
    return 0;
}
