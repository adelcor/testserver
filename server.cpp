#include <iostream>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

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
        
        // Imprimir la solicitud del cliente
        std::cout << "Solicitud del cliente: " << buffer << std::endl;
        
        // Enviar una respuesta al cliente
        const char* message = "Hola, cliente!";
        int message_length = strlen(message);
        int send_result = send(client_sock, message, message_length, 0);
        if (send_result != message_length) {
            std::cout << "Error al enviar respuesta al cliente" << std::endl;
        }
        
        // Cerrar el socket del cliente
        close(client_sock);
    }
    
    // Cerrar el socket del servidor
    close(server_sock);
    
    return 0;
}

