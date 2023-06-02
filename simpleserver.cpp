#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

const int BUFFER_SIZE = 1024;

int main() {
    // Crear el socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Error al crear el socket." << std::endl;
        return 1;
    }

    // Configurar la dirección del servidor
    sockaddr_in serverAddress;
	memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(8080);  // Puerto 8080

    // Enlazar el socket a la dirección del servidor
    if (bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress)) == -1) {
        std::cerr << "Error al enlazar el socket." << std::endl;
        close(serverSocket);
        return 1;
    }

    // Escuchar las conexiones entrantes
    if (listen(serverSocket, 1) == -1) {
        std::cerr << "Error al escuchar en el socket." << std::endl;
        close(serverSocket);
        return 1;
    }

    std::cout << "Servidor iniciado. Esperando conexiones..." << std::endl;

    while (true) {
        // Aceptar una conexión entrante
        sockaddr_in clientAddress;
		memset(&serverAddress, 0, sizeof(serverAddress));
        socklen_t clientAddressLength = sizeof(clientAddress);
        int clientSocket = accept(serverSocket, reinterpret_cast<sockaddr*>(&clientAddress), &clientAddressLength);
        if (clientSocket == -1) {
            std::cerr << "Error al aceptar la conexión." << std::endl;
            close(serverSocket);
            return 1;
        }

        // Leer la petición del cliente
        char buffer[BUFFER_SIZE];
        ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesRead == -1) {
            std::cerr << "Error al leer la petición." << std::endl;
            close(clientSocket);
            continue;
        }

        // Agregar un terminador nulo al final del buffer
        buffer[bytesRead] = '\0';

        // Mostrar la petición recibida
        std::cout << "Petición recibida:" << std::endl;
        std::cout << buffer << std::endl;

        // Responder al cliente
        std::string response = "HTTP/1.1 200 OK\r\n"
                               "Content-Type: text/plain\r\n"
                               "\r\n"
                               "Hola desde el servidor!";
        ssize_t bytesSent = send(clientSocket, response.c_str(), response.size(), 0);
        if (bytesSent == -1) {
            std::cerr << "Error al enviar la respuesta." << std::endl;
        }

        // Cerrar la conexión con el cliente
        close(clientSocket);
    }

    // Cerrar el socket del servidor
    close(serverSocket);

    return 0;
}

