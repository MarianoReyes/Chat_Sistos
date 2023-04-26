# Chat_Sistos

### Compilar Servidor y Cliente

- gcc servidor.c chat-2.pb-c.c -o servidor -lprotobuf-c
- gcc cliente.c chat-2.pb-c.c -o cliente -lprotobuf-c

### Correr Servidor y Cliente

- ./servidor 8080
- ./cliente localhost 8080
