#include <iostream>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0); // Crear un socket TCP
    
    struct sockaddr_in server_address; // Estructura para almacenar la dirección del servidor
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET; // Familia de protocolos IPv4
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1"); // Dirección IP del servidor
    server_address.sin_port = htons(8080); // Puerto del servidor
    
    // Conectar al servidor
    int connect_result = connect(sock, (struct sockaddr*) &server_address, sizeof(server_address));
    if (connect_result < 0) {
        std::cout << "Error al conectarse al servidor" << std::endl;
        return 1;
    }
    
    // Enviar un mensaje al servidor
    const char* message = "Hola, servidor!";
    int message_length = strlen(message);
    int send_result = send(sock, message, message_length, 0);
    if (send_result != message_length) {
        std::cout << "Error al enviar mensaje" << std::endl;
        return 1;
    }
    
    // Recibir la respuesta del servidor
    char buffer[1024];
    int buffer_length = sizeof(buffer);
    int recv_result = recv(sock, buffer, buffer_length, 0);
    if (recv_result < 0) {
        std::cout << "Error al recibir respuesta del servidor" << std::endl;
        return 1;
    }
    
    // Imprimir la respuesta del servidor
    std::cout << "Respuesta del servidor: " << buffer << std::endl;
    
    // Cerrar el socket
    close(sock);
    
    return 0;
}

