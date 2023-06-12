#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <vector>

const int BUFFER_SIZE = 1024;

// Función para establecer un socket no bloqueante
void setNonBlocking(int sockfd) {
    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
}

int main() {
    int serverSocket, clientSocket, maxSockets;
    struct sockaddr_in serverAddress, clientAddress;
    socklen_t clientAddressSize = sizeof(clientAddress);
    std::vector<int> clientSockets;
    char buffer[BUFFER_SIZE];

    // Crear socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Error al crear el socket." << std::endl;
        return 1;
    }

    // Configurar dirección del servidor
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(8080);

    // Enlazar socket a la dirección del servidor
    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1) {
        std::cerr << "Error al enlazar el socket." << std::endl;
        close(serverSocket);
        return 1;
    }

    // Escuchar conexiones entrantes
    if (listen(serverSocket, 10) == -1) {
        std::cerr << "Error al escuchar en el socket." << std::endl;
        close(serverSocket);
        return 1;
    }

    std::cout << "Servidor iniciado. Esperando conexiones..." << std::endl;

    // Establecer socket no bloqueante
    setNonBlocking(serverSocket);

    // Mantener la ejecución del servidor
    while (true) {
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(serverSocket, &readSet);

        maxSockets = serverSocket;

        for (const auto& socket : clientSockets) {
            FD_SET(socket, &readSet);
            maxSockets = std::max(maxSockets, socket);
        }

        // Seleccionar sockets listos para lectura
        if (select(maxSockets + 1, &readSet, NULL, NULL, NULL) == -1) {
            std::cerr << "Error en select." << std::endl;
            close(serverSocket);
            return 1;
        }

        // Manejar nueva conexión entrante
        if (FD_ISSET(serverSocket, &readSet)) {
            clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientAddressSize);
            if (clientSocket == -1) {
                std::cerr << "Error al aceptar la conexión entrante." << std::endl;
            } else {
                std::cout << "Nueva conexión aceptada. Socket: " << clientSocket << std::endl;

                // Establecer socket no bloqueante para el nuevo cliente
                setNonBlocking(clientSocket);
                clientSockets.push_back(clientSocket);
            }
        }

        // Leer datos de los clientes existentes
        for (auto it = clientSockets.begin(); it != clientSockets.end();) {
            if (FD_ISSET(*it, &readSet)) {
                ssize_t bytesRead = recv(*it, buffer, BUFFER_SIZE, 0);
                if (bytesRead == 0) {
                    // Cliente cerró la conexión
                    std::cout << "Conexión cerrada. Socket: " << *it << std::endl;
                    close(*it);
                    it = clientSockets.erase(it);
                } else if (bytesRead == -1) {
                    // Error al recibir datos
                    std::cerr << "Error al recibir datos del socket: " << *it << std::endl;
                    close(*it);
                    it = clientSockets.erase(it);
                } else {
                    // Imprimir la petición recibida
                    std::cout << "Peticion recibida del socket " << *it << ":" << std::endl;
                    std::cout.write(buffer, bytesRead);
                    std::cout << std::endl;

                    // Responder con "Hello World"
                    const char *response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><h1>Hello World!</h1></body></html>";
                    send(*it, response, strlen(response), 0);

                    // Cerrar la conexión después de enviar la respuesta
                    close(*it);
                    it = clientSockets.erase(it);
                }
            } else {
                ++it;
            }
        }
    }

    // Cerrar el socket del servidor
    close(serverSocket);

    return 0;
}

