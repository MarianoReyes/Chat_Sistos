# Chat_Sistos

### Instalar libreria de Protobuf

- sudo apt-get install libprotobuf-c-dev

### Compilar Servidor y Cliente

- gcc servidor.c chat-2.pb-c.c -o servidor -lprotobuf-c
- gcc cliente.c chat-2.pb-c.c -o cliente -lprotobuf-c

### Correr Servidor y Cliente

- ./servidor 8080
- ./cliente localhost 8080
