#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include <ctime>

const int BUFFER_SIZE = 1024;
const int CONTENT_LENGTH_LIMIT = 1024 * 1024;
int serverSocket;

std::string generateFileName() {
    std::time_t now = std::time(nullptr);
    char timestamp[20];
    std::strftime(timestamp, sizeof(timestamp), "%Y%m%d%H%M%S", std::localtime(&now));
    return std::string(timestamp) + ".txt";
}

void handleClient(int clientSocket) {
    char buffer[BUFFER_SIZE];
    std::memset(buffer, 0, BUFFER_SIZE);

    // Leer la solicitud del cliente
    int bytesRead = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
    if (bytesRead < 0) {
        std::cerr << "Error al leer la solicitud del cliente" << std::endl;
        close(clientSocket);
        return;
    }

    // Extraer el nombre del archivo de la solicitud
    std::istringstream iss(buffer);
    std::string requestLine;
    std::getline(iss, requestLine);
    std::string fileName;
    if (requestLine.substr(0, 4) == "POST") {
        fileName = generateFileName();
    } else {
        std::cerr << "Solicitud no válida" << std::endl;
        close(clientSocket);
        return;
    }

    // Abrir el archivo para escritura
    std::ofstream file(fileName.c_str(), std::ios::out | std::ios::binary);
    if (!file) {
        std::cerr << "No se pudo abrir el archivo " << fileName << " para escritura" << std::endl;
        close(clientSocket);
        return;
    }

    // Encontrar el inicio de los datos multipartes
    std::string requestBody(buffer);
    std::size_t pos = requestBody.find("\r\n\r\n");
    if (pos != std::string::npos) {
        pos += 4; // Avanzar hasta después de los delimitadores de inicio de datos

        // Leer y guardar los datos enviados por el cliente
        int requestBodySize = bytesRead - pos;
        file.write(buffer + pos, requestBodySize);

        // Leer el resto de los datos en bucles hasta que se complete la solicitud
        while (true) {
            std::memset(buffer, 0, BUFFER_SIZE);
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
            if (bytesRead <= 0) {
                break;
            }
            file.write(buffer, bytesRead);
            requestBodySize += bytesRead;

            // Verificar si se ha alcanzado el límite de tamaño máximo
            if (requestBodySize >= CONTENT_LENGTH_LIMIT) {
                std::cerr << "Tamaño de datos excede el límite máximo" << std::endl;
                break;
            }
        }
    }

    // Cerrar el archivo
    file.close();

    // Enviar respuesta al cliente
    std::string response = "Archivo recibido exitosamente. Tamaño del archivo: " + std::to_string(file.tellp()) + " bytes\n";
    send(clientSocket, response.c_str(), response.length(), 0);

    // Cerrar el socket del cliente
    close(clientSocket);
}

void signalHandler(int signum) {
    std::cout << "Señal SIGINT recibida. Cerrando el servidor..." << std::endl;
    close(serverSocket);
    exit(signum);
}

int main() {
    int clientSocket;
    struct sockaddr_in serverAddress, clientAddress;
    socklen_t clientLength = sizeof(clientAddress);

    // Crear un socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        std::cerr << "Error al crear el socket" << std::endl;
        return 1;
    }

    // Configurar la dirección del servidor
    std::memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    // Vincular el socket a la dirección del servidor
    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        std::cerr << "Error al vincular el socket" << std::endl;
        return 1;
    }

    // Escuchar conexiones entrantes
    if (listen(serverSocket, 5) < 0) {
        std::cerr << "Error al escuchar conexiones entrantes" << std::endl;
        return 1;
    }

    // Configurar el control de señal para capturar SIGINT
    signal(SIGINT, signalHandler);

    std::cout << "Servidor en funcionamiento. Esperando conexiones..." << std::endl;

    while (true) {
        // Aceptar la conexión del cliente
        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientLength);
        if (clientSocket < 0) {
            std::cerr << "Error al aceptar la conexión del cliente" << std::endl;
            continue;
        }

        // Procesar la solicitud del cliente en un hilo separado
        handleClient(clientSocket);
    }

    return 0;
}

