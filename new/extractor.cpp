#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

void processRequest(const std::string& request) {
    std::cout << "Petición recibida:\n";
    std::cout << request << std::endl;
    std::cout << "----------------------\n";

    // Encontrar el comienzo del contenido multipart
    size_t multipartStart = request.find("\r\n\r\n");
    if (multipartStart != std::string::npos) {
        multipartStart += 4; // Ignorar los caracteres de separación

        // Encontrar la marca del inicio del archivo adjunto
        std::string boundary = request.substr(request.find("boundary=") + 9);
        std::string marker = "--" + boundary;

        // Encontrar la posición del inicio del archivo adjunto
        size_t fileStart = request.find(marker, multipartStart);
        if (fileStart != std::string::npos) {
            fileStart = request.find("\r\n\r\n", fileStart);
            if (fileStart != std::string::npos) {
                fileStart += 4; // Ignorar los caracteres de separación

                // Encontrar la posición del final del archivo adjunto
                size_t fileEnd = request.find(marker, fileStart);
                if (fileEnd != std::string::npos) {
                    // Extraer el contenido del archivo adjunto
                    std::string fileContent = request.substr(fileStart, fileEnd - fileStart);

                    // Mostrar el contenido y el nombre del archivo adjunto
                    std::cout << "Nombre del archivo: archivo_adjunto.txt\n";
                    std::cout << "Contenido del archivo:\n" << fileContent << std::endl;
                }
            }
        }
    }
}

int main() {
    int serverSocket;
    struct sockaddr_in serverAddress;
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
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);  // Cualquier dirección IP

    // Vincular el socket a la dirección del servidor
    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        std::cerr << "Error al vincular el socket." << std::endl;
        return 1;
    }

    // Escuchar por conexiones entrantes
    if (listen(serverSocket, 1) < 0) {
        std::cerr << "Error al escuchar por conexiones entrantes." << std::endl;
        return 1;
    }

    std::cout << "Servidor iniciado. Esperando conexiones..." << std::endl;

    while (true) {
        int clientSocket;
        struct sockaddr_in clientAddress;
        socklen_t clientAddressLength = sizeof(clientAddress);

        // Aceptar una conexión entrante
        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLength);
        if (clientSocket < 0) {
            std::cerr << "Error al aceptar la conexión entrante." << std::endl;
            continue;
        }

        // Leer la solicitud del cliente
        std::string request;
        while (true) {
            memset(buffer, 0, BUFFER_SIZE);
            int bytesRead = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
            if (bytesRead <= 0)
                break;
            request += buffer;
        }

        // Procesar la solicitud
		std::cout << "REQUEST ES: " << request << std::endl;
        processRequest(request);

        // Cerrar la conexión con el cliente
        close(clientSocket);
    }

    // Cerrar el socket del servidor
    close(serverSocket);

    return 0;
}

