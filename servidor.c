#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include "chat-2.pb-c.h"

#define PORT 8080
#define MAX_CONNECTIONS 10
#define BUFFER_SIZE 1024

int main(int argc, char const *argv[]) {
    int server_fd, new_socket, valread, opt = 1;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    // Crear socket servidor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Configurar opciones de socket
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                                                  &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // Configurar dirección de servidor
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Asignar dirección al socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Escuchar conexiones entrantes
    if (listen(server_fd, MAX_CONNECTIONS) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while (true) {
        // Aceptar conexión entrante
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                          (socklen_t*)&addrlen))<0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        // Leer mensaje del cliente
        valread = read(new_socket, buffer, BUFFER_SIZE);

        // Deserializar mensaje recibido
        ChatSistOS__UserOption* user_option = chat_sist_o_s__user_option__unpack(NULL, valread, (const uint8_t*)buffer);
        if (user_option == NULL) {
            perror("chat_sist_o_s__user_option__unpack");
            exit(EXIT_FAILURE);
        }

        // Procesar opción recibida
        ChatSistOS__Answer answer = CHAT_SIST_O_S__ANSWER__INIT;
        answer.op = user_option->op;

        switch (user_option->op) {
            case 1: // Crear nuevo usuario
                // Obtener datos del nuevo usuario
                ChatSistOS__NewUser* new_user = user_option->createuser;
                // Procesar datos y enviar respuesta
                // ...
                break;
            case 2: // Ver usuarios conectados/ver estado de usuario
                // Obtener datos de la solicitud
                ChatSistOS__UserList* user_list = user_option->userlist;
                // Procesar datos y enviar respuesta
                // ...
                break;
            case 3: // Cambiar estado de usuario
                // Obtener datos de la solicitud
                ChatSistOS__Status* status = user_option->status;
                // Procesar datos y enviar respuesta
                // ...
                break;
            case 4: // Enviar mensaje
                // Obtener datos del mensaje
                ChatSistOS__Message* message = user_option->message;

                // Procesar datos y responder
                if (message->message_private) {
                    // Mensaje privado
                    ChatSistOS__Answer answer = CHAT_SISTOS__ANSWER__INIT;
                    answer.op = 4;

                    // Buscar usuario destino
                    int found = 0;
                    for (int i = 0; i < users_online->n_users; i++) {
                        ChatSistOS__User* user = users_online->users[i];
                        if (strcmp(user->user_name, message->message_destination) == 0) {
                            found = 1;
                            // Enviar mensaje al usuario destino
                            ChatSistOS__Answer answer_to_dest = CHAT_SISTOS__ANSWER__INIT;
                            answer_to_dest.op = 5;
                            answer_to_dest.message = message;
                            int message_to_dest_len = chat_sistos__answer__get_packed_size(&answer_to_dest);
                            void* message_to_dest_buf = malloc(message_to_dest_len);
                            chat_sistos__answer__pack(&answer_to_dest, message_to_dest_buf);
                            send(user->socket_fd, message_to_dest_buf, message_to_dest_len, 0);
                            free(message_to_dest_buf);
                            break;
                        }
                    }

                    if (!found) {
                        answer.response_status_code = 400;
                        answer.response_message = "User not found";
                    } else {
                        answer.response_status_code = 200;
                        answer.response_message = "Message sent";
                    }

                    // Enviar respuesta al usuario emisor
                    int answer_len = chat_sistos__answer__get_packed_size(&answer);
                    void* answer_buf = malloc(answer_len);
                    chat_sistos__answer__pack(&answer, answer_buf);
                    send(socket_fd, answer_buf, answer_len, 0);
                    free(answer_buf);

                } else {
                    // Mensaje a todos
                    ChatSistOS__Answer answer = CHAT_SISTOS__ANSWER__INIT;
                    answer.op = 4;
                    answer.response_status_code = 200;
                    answer.response_message = "Message sent to all users";

                    // Enviar mensaje a todos los usuarios
                    for (int i = 0; i < users_online->n_users; i++) {
                        ChatSistOS__User* user = users_online->users[i];
                        if (user->socket_fd != socket_fd) {
                            // Enviar mensaje a los usuarios que no lo envió
                            ChatSistOS__Answer answer_to_user = CHAT_SISTOS__ANSWER__INIT;
                            answer_to_user.op = 5;
                            answer_to_user.message = message;
                            int message_to_user_len = chat_sistos__answer__get_packed_size(&answer_to_user);
                            void* message_to_user_buf = malloc(message_to_user_len);
                            chat_sistos__answer__pack(&answer_to_user, message_to_user_buf);
                            send(user->socket_fd, message_to_user_buf, message_to_user_len, 0);
                            free(message_to_user_buf);
                        }
                    }

                    // Enviar respuesta al usuario emisor
                    int answer_len = chat_sist_os__answer__get_packed_size(&answer);
                    void* answer_buf = malloc(answer_len);
                    chat_sist_os__answer__pack(&answer, answer_buf);

                    // Enviar respuesta al usuario emisor
                    send(client_socket, answer_buf, answer_len, 0);

                    // Liberar memoria
                    free(user_option_buf);
                    free(answer_buf);
                    chat_sist_os__user_option__free_unpacked(user_option, NULL);
                    chat_sist_os__answer__free_unpacked(&answer, NULL);
                }

                // Cerrar conexión con el cliente
                close(client_socket);
            }

        // Cerrar socket del servidor
        close(server_socket);

    return 0;
}

