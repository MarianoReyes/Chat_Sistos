#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <pthread.h>
#include "chat-2.pb-c.h"

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define PORT 8082

typedef struct
{
    int client_fd;
    char username[BUFFER_SIZE];
    int user_state;
    pthread_t thread_id;
} client_t;

client_t clients[MAX_CLIENTS];
int num_clients = 0;

void *client_handler(void *arg)
{
    client_t *client = (client_t *)arg;
    char buffer[BUFFER_SIZE];
    int bytes_received, bytes_sent;
    int client_fd = client->client_fd;

    ChatSistOS__Answer answer = CHAT_SIST_OS__ANSWER__INIT;
    int answer_size;

    // Recibimos mensajes del cliente
    while ((bytes_received = recv(client_fd, buffer, BUFFER_SIZE, 0)) > 0)
    {
        // Procesamos el mensaje
        ChatSistOS__UserOption *user_option = chat_sist_os__user_option__unpack(NULL, bytes_received, buffer);
        if (user_option == NULL)
        {
            perror("Error al decodificar el mensaje");
            break;
        }

        // Manejamos la opción
        switch (user_option->op)
        {
        case 1:
            // Crear nuevo usuario
            strcpy(client->username, user_option->createuser->username);

            // Creamos el mensaje de respuesta de usuario
            answer.response_message = "Usuario creado con exito.";
            answer.op = 1;
            answer_size = chat_sist_os__answer__get_packed_size(&answer);
            chat_sist_os__answer__pack(&answer, buffer);

            // Enviamos el mensaje al cliente
            bytes_sent = send(client_fd, buffer, answer_size, 0);
            if (bytes_sent < 0)
            {
                perror("Error al enviar el mensaje al cliente");
            }

            printf("Nuevo usuario creado: %s\n", client->username);
            break;
        case 2:
            // Ver usuarios conectados
            printf("Usuario %s solicitó lista de usuarios conectados\n", client->username);
            printf("Lista de usuarios conectados:\n");
            for (int i = 0; i < num_clients; i++)
            {
                if (clients[i].thread_id != 0)
                {
                    printf("- %s\n", clients[i].username);
                }
            }

            // Creamos el mensaje de respuesta de usuario
            answer.response_message = "Lista de usuarios desplegada.";
            answer.op = 2;
            answer_size = chat_sist_os__answer__get_packed_size(&answer);
            chat_sist_os__answer__pack(&answer, buffer);

            // Enviamos el mensaje al cliente
            bytes_sent = send(client_fd, buffer, answer_size, 0);
            if (bytes_sent < 0)
            {
                perror("Error al enviar el mensaje al cliente");
            }

            break;
        case 3:
            if (user_option->status != NULL && strlen(user_option->status->user_name) > 0){
                for (int i = 0; i < num_clients; i++)
                {
                    if (strcmp(clients[i].username, user_option->status->user_name) == 0)
                    {
                        // Mostrar informacion
                        char estado[20] = "";
                        if ((clients[i].user_state)==1){
                            strcpy(estado, "conectado");
                        }
                        else if((clients[i].user_state)==2){
                            strcpy(estado, "ocupado");
                        }
                        else{
                            strcpy(estado, "desconectado");
                        }

                        char informacion[30];

                        strcpy(informacion, clients[i].username); 
                        strcat(informacion, " esta: "); 
                        strcat(informacion, estado); 

                        // Mostramos info del usuario
                        answer.response_message = informacion;
                        answer.op = 3;
                        answer_size = chat_sist_os__answer__get_packed_size(&answer);
                        chat_sist_os__answer__pack(&answer, buffer);

                        // Enviamos el mensaje al cliente
                        bytes_sent = send(client_fd, buffer, answer_size, 0);
                        if (bytes_sent < 0)
                        {
                            perror("Error al enviar el mensaje al cliente");
                        }

                        printf("Usuario %s tiene su estado como: %s\n", clients[i].username, estado);
                    }
                }
            }
            else{
                // Cambiar estado del usuario
                client->user_state = user_option->status->user_state;

                // Creamos el mensaje de respuesta de usuario
                answer.response_message = "Estado cambiado con exito.";
                answer.op = 3;
                answer_size = chat_sist_os__answer__get_packed_size(&answer);
                chat_sist_os__answer__pack(&answer, buffer);

                // Enviamos el mensaje al cliente
                bytes_sent = send(client_fd, buffer, answer_size, 0);
                if (bytes_sent < 0)
                {
                    perror("Error al enviar el mensaje al cliente");
                }

                printf("Usuario %s cambió su estado a %d\n", client->username, client->user_state);
            }
                
            break;
        case 4:
            // Enviar mensaje
            ChatSistOS__Message message = CHAT_SIST_OS__MESSAGE__INIT;

            if (user_option->message->message_private == 0){
            // broadcast
                for (int i = 0; i < num_clients; i++)
                {       
                    message.message_sender = user_option->message->message_sender;
                    message.message_content = user_option->message->message_content;

                    // Creamos el mensaje de respuesta de usuario
                    answer.message = &message;
                    answer.op = 4;
                    answer_size = chat_sist_os__answer__get_packed_size(&answer);
                    chat_sist_os__answer__pack(&answer, buffer);

                    // Enviamos el mensaje al cliente
                    bytes_sent = send(client_fd, buffer, answer_size, 0);
                    if (bytes_sent < 0)
                    {
                        perror("Error al enviar el mensaje al cliente");
                    }

                    printf("Usuario %s mando el siguiente mensaje como broadcast: %s\n", user_option->message->message_sender, user_option->message->message_content);
                    break;
                }
            } 
            else{
            // privado
                for (int i = 0; i < num_clients; i++)
                {
                    if (strcmp(clients[i].username, user_option->message->message_destination) == 0)
                    {
                        int dest_fd = clients[i].client_fd;
                        ChatSistOS__UserOption option = CHAT_SIST_OS__USER_OPTION__INIT;
                        option.op = 4;
                        ChatSistOS__Message message = CHAT_SIST_OS__MESSAGE__INIT;
                        message.message_sender = user_option->message->message_sender;
                        message.message_destination = user_option->message->message_destination;
                        message.message_content = user_option->message->message_content;
                        option.message = &message;
                        size_t size = chat_sist_os__user_option__get_packed_size(&option);
                        uint8_t *buffer = malloc(size);
                        chat_sist_os__user_option__pack(&option, buffer);
                        send(dest_fd, buffer, size, 0);
                        free(buffer);
                        break;
                    }
                }
            }

            // Creamos el mensaje de respuesta de usuario
            answer.response_message = "Mensaje enviado con exito.";
            answer_size = chat_sist_os__answer__get_packed_size(&answer);
            chat_sist_os__answer__pack(&answer, buffer);

            // Enviamos el mensaje al cliente
            bytes_sent = send(client_fd, buffer, answer_size, 0);
            if (bytes_sent < 0)
            {
                perror("Error al enviar el mensaje al cliente");
            }
            printf("Usuario %s mando un mensaje\n", client->username);
            break;
        default:
            // Opción inválida
            printf("Usuario %s envió una opción inválida: %d\n", client->username, user_option->op);
            break;
        }
        chat_sist_os__user_option__free_unpacked(user_option, NULL);
    }

    // El cliente se desconectó
    printf("Usuario %s se desconectó\n", client->username);
    close(client_fd);
    memset(client->username, 0, BUFFER_SIZE);
    client->thread_id = 0;
    pthread_exit(NULL);
}

void add_client(int client_fd)
{
    if (num_clients >= MAX_CLIENTS)
    {
        printf("Máximo número de clientes alcanzado\n");
        close(client_fd);
        return;
    }

    client_t *client = &clients[num_clients];
    client->client_fd = client_fd;
    client->thread_id = 0;
    num_clients++;

    printf("Nuevo usuario conectado\n");
}

void remove_client(int index)
{
    client_t *client = &clients[index];
    close(client->client_fd);
    memset(client->username, 0, BUFFER_SIZE);
    client->thread_id = 0;
    num_clients--;

    printf("Usuario desconectado\n");

    // Si no es el último cliente de la lista, movemos el último cliente a su posición
    if (index < num_clients)
    {
        clients[index] = clients[num_clients];
    }
}

int main(int argc, char *argv[])
{
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    pthread_t thread_id;

    // Creamos el socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
    {
        perror("Error al crear el socket");
        exit(EXIT_FAILURE);
    }

    // Configuramos la dirección del servidor
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Asociamos el socket con la dirección del servidor
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("Error al asociar el socket con la dirección del servidor");
        exit(EXIT_FAILURE);
    }

    // Escuchamos por nuevas conexiones
    if (listen(server_fd, MAX_CLIENTS) == -1)
    {
        perror("Error al escuchar por nuevas conexiones");
        exit(EXIT_FAILURE);
    }

    printf("Servidor iniciado. Esperando conexiones...\n");

    // Manejamos conexiones entrantes
    while (1)
    {
        // Aceptamos la conexión entrante
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_fd == -1)
        {
            perror("Error al aceptar la conexión entrante");
            continue;
        }

        printf("Nueva conexión entrante\n");

        // Agregamos el cliente a la lista
        add_client(client_fd);

        // Creamos un hilo para manejar al cliente
        client_t *client = &clients[num_clients - 1];
        if (pthread_create(&client->thread_id, NULL, client_handler, (void *)client) != 0)
        {
            perror("Error al crear el hilo");
            remove_client(num_clients - 1);
        }
    }

    // Cerramos el socket del servidor
    close(server_fd);

    return 0;
}
