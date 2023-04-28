#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "chat-2.pb-c.h"

#define BUFFER_SIZE 1024
#define PORT 8082

int sockfd = 0;

void print_menu()
{
    printf("\nSeleccione una opción:\n");
    printf("1. Crear nuevo usuario\n");
    printf("2. Ver usuarios conectados\n");
    printf("3. Cambiar estado\n");
    printf("4. Ver informacion de algun usuario\n");
    printf("5. Enviar mensaje\n");
    printf("6. Enviar broadcast\n");
    printf("7. Salir\n");
}

void create_user()
{
    ChatSistOS__NewUser createuser = CHAT_SIST_OS__NEW_USER__INIT;
    ChatSistOS__UserOption user_option = CHAT_SIST_OS__USER_OPTION__INIT;
    char buffer[BUFFER_SIZE];
    int bytes_sent;

    // Leemos el nombre de usuario 
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
    bytes_sent = send(sockfd, option_buffer, option_size, 0);
    if (bytes_sent < 0)
    {
        perror("Error al enviar el mensaje al servidor");
        return;
    }

}

void view_users()
{
    ChatSistOS__UserOption user_option = CHAT_SIST_OS__USER_OPTION__INIT;
    char buffer[BUFFER_SIZE];
    int bytes_sent;

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

}

void change_status()
{
    ChatSistOS__Status status = CHAT_SIST_OS__STATUS__INIT;
    ChatSistOS__UserOption user_option = CHAT_SIST_OS__USER_OPTION__INIT;
    char buffer[BUFFER_SIZE];
    int bytes_sent;

    // Leemos el nuevo estado
    printf("Seleccione un estado (1 = conectado, 2 = ocupado, 3 = desconectado): ");
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

}

void user_info()
{
    ChatSistOS__Status status = CHAT_SIST_OS__STATUS__INIT;
    ChatSistOS__UserOption user_option = CHAT_SIST_OS__USER_OPTION__INIT;
    char buffer[BUFFER_SIZE];
    int bytes_sent;

    // Leemos el nombre de usuario 
    printf("Introduzca un nombre de usuario para recibir infomacion: ");
    char username[BUFFER_SIZE];
    fgets(username, BUFFER_SIZE, stdin);
    username[strlen(username) - 1] = '\0';
    status.user_name = username;

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

}

void send_message()
{
    ChatSistOS__Message message = CHAT_SIST_OS__MESSAGE__INIT;
    ChatSistOS__UserOption user_option = CHAT_SIST_OS__USER_OPTION__INIT;
    char buffer[BUFFER_SIZE];
    int bytes_sent;

    // Leemos el destinatario
    printf("Introduzca el nombre del destinatario: ");
    char message_dest[BUFFER_SIZE];
    fgets(message_dest, BUFFER_SIZE, stdin);
    message_dest[strlen(message_dest) - 1] = '\0';
    message.message_destination = message_dest;

    // Leemos el mensaje
    printf("Introduzca el mensaje: ");
    char message_cont[BUFFER_SIZE];
    fgets(message_cont, BUFFER_SIZE, stdin);
    message_cont[strlen(message_cont) - 1] = '\0';
    message.message_content = message_cont;

    // Seteamos visibilidad de mensaje
    message.message_private = 1;

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

}

void send_broadcast()
{
    ChatSistOS__Message message = CHAT_SIST_OS__MESSAGE__INIT;
    ChatSistOS__UserOption user_option = CHAT_SIST_OS__USER_OPTION__INIT;
    char buffer[BUFFER_SIZE];
    int bytes_sent;

    // Leemos el mensaje
    printf("Introduzca el mensaje: ");
    char message_cont[BUFFER_SIZE];
    fgets(message_cont, BUFFER_SIZE, stdin);
    message_cont[strlen(message_cont) - 1] = '\0';
    message.message_content = message_cont;

    // Seteamos visibilidad de mensaje
    message.message_private = 0;

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
}

void* receive_messages(void* arg)
{
    // Thread que se ejecuta en segundo plano para recibir todas las respuestass
    char buffer[BUFFER_SIZE];
    int bytes_received;

    while (true)
    {
        // Esperamos la respuesta del servidor
        bytes_received = recv(sockfd, buffer, BUFFER_SIZE, 0);
        if (bytes_received < 0)
        {
            perror("Error al recibir la respuesta del servidor");
            return NULL;
        }

        // Decodificamos la respuesta
        ChatSistOS__Answer *response_message = chat_sist_os__answer__unpack(NULL, bytes_received, buffer);
        if (response_message == NULL)
        {
            perror("Error al decodificar la respuesta del servidor");
        }

        // Imprimimos el mensaje recibido
        if ((response_message->op == 1 || response_message->op == 2 || response_message->op == 3)) {
            printf("\n%s\n", response_message->response_message);
        }
        else if((response_message->op)==4){ // AREGLAR ESTO
            printf("\nNuevo mensaje de %s: %s\n", response_message->message->message_sender, response_message->message->message_sender);
        }

        chat_sist_os__answer__free_unpacked(response_message, NULL);
    }
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

    // Creamos el hilo para recibir mensajes del servidor
    pthread_t receive_thread;
    if (pthread_create(&receive_thread, NULL, receive_messages, NULL) < 0)
    {
        perror("Error al crear el hilo para recibir mensajes del servidor");
        return 1;
    }

    // Hacemos el detach del hilo
    pthread_detach(receive_thread);

    while (1)
    {
        usleep(500000);
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
            user_info();
            break;
        case 5:
            send_message();
            break;
        case 6:
            send_broadcast();
            break;
        case 7:
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
