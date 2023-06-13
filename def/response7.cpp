#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fstream>
#include <cstring>

std::string findBinary(const std::string& cadena, const std::string& separador) {
    std::string::size_type pos = cadena.find(separador);
    std::string::size_type lastPos = std::string::npos;

    while (pos != std::string::npos) {
        lastPos = pos;
        pos = cadena.find(separador, pos + 1);
    }

    if (lastPos != std::string::npos) {
        return cadena.substr(lastPos + separador.length());
    }

    return "";
}

std::string cut(const std::string& cadena, const std::string& separador) {
    std::size_t pos = cadena.find(separador);
    if (pos != std::string::npos) {
        return cadena.substr(0, pos);
    }
    return cadena;
}

std::string generateResponse(const std::string& request) {
    std::string response;

    if (request.find("GET") == 0) {
        response = "HTTP/1.1 200 OK\r\n";
        response += "Content-Type: text/plain\r\n";
        response += "Content-Length: 23\r\n";
        response += "\r\n";
        response += "Peticion GET procesada!";
    } else if (request.find("POST") == 0) {
        response = "HTTP/1.1 200 OK\r\n";
        response += "Content-Type: text/plain\r\n";
        response += "Content-Length: 24\r\n";
        response += "\r\n";
        response += "Peticion POST procesada!";
    } else {
        response = "HTTP/1.1 400 Bad Request\r\n";
        response += "Content-Type: text/plain\r\n";
        response += "Content-Length: 11\r\n";
        response += "\r\n";
        response += "Bad Request";
    }

    return response;
}

int main() {
    // Crear el socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    std::string requestHeader;
    std::string requestBody;
    std::string binary;

    if (serverSocket < 0) {
        std::cerr << "Error al crear el socket" << std::endl;
        return 1;
    }

    // Configurar la dirección del servidor
    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddress.sin_port = htons(8080); // Puerto 8080

    // Vincular el socket a la dirección del servidor
    if (bind(serverSocket, reinterpret_cast<struct sockaddr *>(&serverAddress), sizeof(serverAddress)) < 0) {
        std::cerr << "Error al vincular el socket" << std::endl;
        close(serverSocket);
        return 1;
    }

    // Escuchar nuevas conexiones
    if (listen(serverSocket, 1) < 0) {
        std::cerr << "Error al escuchar en el socket" << std::endl;
        close(serverSocket);
        return 1;
    }

    std::cout << "Servidor a la escucha en el puerto 8080..." << std::endl;

    while (true) {
        int clientSocket;

        // Aceptar una conexión entrante
        clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket < 0) {
            std::cerr << "Error al aceptar la conexión" << std::endl;
            close(serverSocket);
            return 1;
        }

        // Leer los datos de la solicitud
        char c;
        ssize_t bytesRead;
        int contentLength = 0;
        bool foundEndOfHeaders = false;
        std::string endOfHeaders = "\r\n\r\n";
        std::string contentLengthHeader = "Content-Length: ";

        while ((bytesRead = recv(clientSocket, &c, 1, 0)) > 0) {
            if (!foundEndOfHeaders) {
                requestHeader.push_back(c);

                if (requestHeader.find(endOfHeaders) != std::string::npos) {
                    foundEndOfHeaders = true;

                    // Obtener la longitud del contenido del cuerpo de la solicitud
                    std::size_t contentLengthPos = requestHeader.find(contentLengthHeader);
                    if (contentLengthPos != std::string::npos) {
                        contentLengthPos += contentLengthHeader.length();
                        std::string contentLengthStr = cut(requestHeader.substr(contentLengthPos), "\r\n");
                        contentLength = std::stoi(contentLengthStr);
                    }
                }
            } else {
                requestBody.push_back(c);
            }

            // Verificar si se ha leído el contenido completo
            if (foundEndOfHeaders && requestBody.length() >= contentLength) {
                break;
            }
        }

        std::cout << "Solicitud recibida:" << std::endl;
        std::cout << requestHeader << std::endl;
        std::cout << requestBody << std::endl;

        // Extraer el contenido binario de la solicitud
        binary = findBinary(requestBody, "\r\n\r\n");

        std::ofstream outputFile("archivo.jpg", std::ios::binary);
        outputFile.write(binary.data(), binary.size());
        outputFile.close();

        std::string response = generateResponse(requestHeader);
        send(clientSocket, response.c_str(), response.length(), 0);

        // Limpiar los datos y cerrar el socket del cliente
        requestHeader.clear();
        requestBody.clear();
        close(clientSocket);
    }

    // Cerrar el socket del servidor
    close(serverSocket);

    return 0;
}

