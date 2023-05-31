#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

// Función para mostrar los datos recibidos
void processRequest(const std::string& request) {
    std::cout << "Petición recibida:\n";
    std::cout << request << std::endl;
    std::cout << "----------------------\n";
}

int main() {
    int serverSocket, clientSocket;
    socklen_t clientAddressLength;
    struct sockaddr_in serverAddress, clientAddress;
    char buffer[BUFFER_SIZE];

    // Crear socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        std::cerr << "Error al crear el socket." << std::endl;
        return 1;
    }

    // Configurar la dirección del servidor
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);  // Puerto 8080
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    // Vincular el socket a la dirección del servidor
    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        std::cerr << "Error al vincular el socket." << std::endl;
        return 1;
    }

    // Escuchar por conexiones entrantes
    if (listen(serverSocket, 5) < 0) {
        std::cerr << "Error al intentar escuchar por conexiones entrantes." << std::endl;
        return 1;
    }

    std::cout << "Servidor iniciado. Esperando conexiones..." << std::endl;

    while (true) {
        // Aceptar una conexión entrante
        clientAddressLength = sizeof(clientAddress);
        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLength);
        if (clientSocket < 0) {
            std::cerr << "Error al aceptar la conexión." << std::endl;
            return 1;
        }

        // Leer la petición del cliente
        memset(buffer, 0, BUFFER_SIZE);
        std::string request;

        while (true) {
            int bytesRead = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
            if (bytesRead <= 0)
                break;
            request += buffer;

            if (request.find("\r\n\r\n") != std::string::npos)
                break;

            memset(buffer, 0, BUFFER_SIZE);
        }

        // Procesar la petición recibida
        processRequest(request);

        // Responder al cliente
        std::string response = "HTTP/1.1 200 OK\r\n";
        response += "Content-Type: text/plain\r\n";
        response += "Connection: close\r\n\r\n";
        response += "Petición recibida correctamente.\r\n";

        send(clientSocket, response.c_str(), response.length(), 0);

        // Cerrar la conexión con el cliente
        close(clientSocket);
    }

    // Cerrar el socket del servidor
    close(serverSocket);

    return 0;
}

