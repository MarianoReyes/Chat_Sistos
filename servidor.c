#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "chat-2.pb-c.h"

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define PORT 8080

typedef struct
{
    int client_fd;
    char username[BUFFER_SIZE];
    pthread_t thread_id;
} client_t;

client_t clients[MAX_CLIENTS];
int num_clients = 0;

void *client_handler(void *arg)
{
    client_t *client = (client_t *)arg;
    char buffer[BUFFER_SIZE];
    int bytes_received;
    int client_fd = client->client_fd;

    // Recibimos mensajes del cliente
    while ((bytes_received = recv(client_fd, buffer, BUFFER_SIZE, 0)) > 0)
    {
        // Procesamos el mensaje
        ChatSistOS__UserOption *user_option = chat_sist_o_s__user_option__unpack(NULL, bytes_received, buffer);
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
            break;
        case 3:
            // Cambiar estado del usuario
            client->user_state = user_option->status->user_state;
            printf("Usuario %s cambió su estado a %d\n", client->username, client->user_state);
            break;
        case 4:
            // Enviar mensaje
            char *destination = user_option->message->message_destination;
            char *sender = user_option->message->message_sender;
            char *content = user_option->message->message_content;
            for (int i = 0; i < num_clients; i++)
            {
                if (strcmp(clients[i].username, destination) == 0)
                {
                    int dest_fd = clients[i].client_fd;
                    ChatSistOS__UserOption option = CHAT_SIST_O_S__USER_OPTION__INIT;
                    option.op = 4;
                    ChatSistOS__Message message = CHAT_SIST_O_S__MESSAGE__INIT;
                    message.message_sender = sender;
                    message.message_destination = destination;
                    message.message_content = content;
                    option.message = &message;
                    size_t size = chat_sist_o_s__user_option__get_packed_size(&option);
                    uint8_t *buffer = malloc(size);
                    chat_sist_o_s__user_option__pack(&option, buffer);
                    send(dest_fd, buffer, size, 0);
                    free(buffer);
                    break;
                }
            }
            break;
        default:
            // Opción inválida
            printf("Usuario %s envió una opción inválida: %d\n", client->username, user_option->op);
            break;
        }

        chat_sist_o_s__user_option__free_unpacked(user_option, NULL);
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

    printf("Usuario desconectado\n")

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
