#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>

int main() {
    // Crear el socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Error al crear el socket." << std::endl;
        return 1;
    }

    // Configurar la dirección y el puerto para enlazar el socket
    sockaddr_in serverAddress={};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8081);  // Puerto 8081
    serverAddress.sin_addr.s_addr = inet_addr("10.14.6.5");

    // Enlazar el socket
    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        std::cerr << "Error al enlazar el socket." << std::endl;
        close(serverSocket);
        return 1;
    }

    // Escuchar el socket
    if (listen(serverSocket, 10) == -1) {
        std::cerr << "Error al escuchar el socket." << std::endl;
        close(serverSocket);
        return 1;
    }

    std::cout << "Esperando conexiones entrantes..." << std::endl;

    while (true) {
        // Aceptar una conexión entrante
        sockaddr_in clientAddress={};
        socklen_t clientAddressLength = sizeof(clientAddress);
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLength);
        if (clientSocket == -1) {
            std::cerr << "Error al aceptar la conexión." << std::endl;
            close(serverSocket);
            return 1;
        }

        std::cout << "Se ha establecido una nueva conexión." << std::endl;

        // Leer datos del cliente
        char buffer[1024];
        ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead == -1) {
            std::cerr << "Error al leer los datos del cliente." << std::endl;
            close(clientSocket);
            close(serverSocket);
            return 1;
        }

        // Imprimir los datos recibidos
        std::cout << "Datos recibidos del cliente:" << std::endl;
        std::cout.write(buffer, bytesRead);
        std::cout << std::endl;

        // Enviar una respuesta al cliente
        const char* response = "HTTP/1.1 200 OK\r\nContent-Length: 12\r\n\r\nHello World!";
        ssize_t bytesSent = send(clientSocket, response, strlen(response), 0);
        if (bytesSent == -1) {
            std::cerr << "Error al enviar la respuesta al cliente." << std::endl;
            close(clientSocket);
            close(serverSocket);
            return 1;
        }

        // Cerrar el socket del cliente
        close(clientSocket);
        std::cout << "Conexión cerrada." << std::endl;
    }

    // Cerrar el socket del servidor
    close(serverSocket);
    std::cout << "socket cerrado\n" << std::endl;

    return 0;
}

