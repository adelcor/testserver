#include <iostream>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>

int main() {
    int server_sock = socket(AF_INET, SOCK_STREAM, 0); // Crear un socket TCP

    struct sockaddr_in server_address; // Estructura para almacenar la dirección del servidor
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET; // Familia de protocolos IPv4
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1"); // Dirección IP del servidor
    server_address.sin_port = htons(8080); // Puerto del servidor

    // Asignar dirección al socket
    int bind_result = bind(server_sock, (struct sockaddr*) &server_address, sizeof(server_address));
    if (bind_result < 0) {
        std::cout << "Error al asignar dirección al socket" << std::endl;
        return 1;
    }

    // Configurar el socket para aceptar conexiones entrantes
    int listen_result = listen(server_sock, 5); // 5 conexiones en cola
    if (listen_result < 0) {
        std::cout << "Error al configurar el socket para escuchar conexiones entrantes" << std::endl;
        return 1;
    }

    std::cout << "Servidor iniciado en el puerto 8080" << std::endl;

    while (true) {
        struct sockaddr_in client_address; // Estructura para almacenar la dirección del cliente
        socklen_t client_address_length = sizeof(client_address);
        int client_sock = accept(server_sock, (struct sockaddr*) &client_address, &client_address_length); // Aceptar una conexión entrante
        if (client_sock < 0) {
            std::cout << "Error al aceptar conexión entrante" << std::endl;
            continue;
        }

        // Leer la solicitud del cliente
        char buffer[1024];
        int buffer_length = sizeof(buffer);
        int recv_result = recv(client_sock, buffer, buffer_length, 0);
        if (recv_result < 0) {
            std::cout << "Error al leer solicitud del cliente" << std::endl;
            continue;
        }

        // Analizar la solicitud del cliente para determinar si es una
// solicitud GET
if (strncmp(buffer, "GET ", 4) == 0) {
// Obtener la URL solicitada
char* url_start = buffer + 4;
char* url_end = strstr(url_start, " HTTP/1.1\r\n");
if (url_end == nullptr) {
std::cout << "Error al analizar la URL solicitada" << std::endl;
continue;
}
*url_end = '\0';
std::cout << "Solicitud GET recibida para la URL: " << url_start << std::endl;

        // Abrir el archivo correspondiente a la URL solicitada
        FILE* file = fopen(url_start, "r");
        if (file == nullptr) {
            // Si el archivo no existe, enviar una respuesta 404 Not Found
            const char* response = "HTTP/1.1 404 Not Found\r\nContent-Length: 15\r\n\r\n404 Not Found\n";
            send(client_sock, response, strlen(response), 0);
        } else {
            // Si el archivo existe, enviar una respuesta 200 OK junto con el contenido del archivo
            const char* response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
            send(client_sock, response, strlen(response), 0);

            char file_buffer[1024];
            size_t read_size;
            while ((read_size = fread(file_buffer, 1, sizeof(file_buffer), file)) > 0) {
                send(client_sock, file_buffer, read_size, 0);
            }

            fclose(file);
        }
    } else {
        // Si la solicitud no es GET, enviar una respuesta 501 Not Implemented
        const char* response = "HTTP/1.1 501 Not Implemented\r\nContent-Length: 19\r\n\r\n501 Not Implemented\n";
        send(client_sock, response, strlen(response), 0);
    }

    // Cerrar el socket del cliente
    close(client_sock);
}

// Cerrar el socket del servidor
close(server_sock);

return 0;
}
