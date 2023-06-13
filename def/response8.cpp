#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fstream>
#include <cstring>
#include <sys/select.h>

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
    std::string data;
    std::string binary;
    std::string temp;
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

    // Conjunto de descriptores de sockets
    fd_set readSockets;
    int maxSocket = serverSocket;

    while (true) {
        // Limpiar el conjunto de descriptores de sockets y agregar el socket del servidor
        FD_ZERO(&readSockets);
        FD_SET(serverSocket, &readSockets);

        // Agregar los sockets de los clientes conectados
        for (int i = serverSocket + 1; i <= maxSocket; ++i) {
            FD_SET(i, &readSockets);
        }

        // Esperar por actividad en algún socket
        int activity = select(maxSocket + 1, &readSockets, nullptr, nullptr, nullptr);
        if (activity < 0) {
            std::cerr << "Error en select" << std::endl;
            close(serverSocket);
            return 1;
        }

        // Verificar si hay una nueva conexión entrante
        if (FD_ISSET(serverSocket, &readSockets)) {
            int clientSocket;

            // Aceptar una conexión entrante
            clientSocket = accept(serverSocket, nullptr, nullptr);
            if (clientSocket < 0) {
                std::cerr << "Error al aceptar la conexión" << std::endl;
                close(serverSocket);
                return 1;
            }

            // Agregar el nuevo socket al conjunto de descriptores de sockets
            FD_SET(clientSocket, &readSockets);

            // Actualizar el máximo descriptor de socket
            if (clientSocket > maxSocket) {
                maxSocket = clientSocket;
            }

            std::cout << "Nueva conexión aceptada. Socket: " << clientSocket << std::endl;
        }

        // Verificar si hay datos disponibles para lectura en algún socket de cliente
        for (int i = serverSocket + 1; i <= maxSocket; ++i) {
            if (FD_ISSET(i, &readSockets)) {
                int clientSocket = i;

                // Leer los datos de la solicitud
                char c;
                ssize_t bytesRead;
                int contentLength = 0;
                bool foundEndOfHeaders = false;
                std::string endOfHeaders = "\r\n\r\n";
                std::string contentLengthHeader = "Content-Length: ";

                while ((bytesRead = recv(clientSocket, &c, 1, 0)) > 0) {
                    data.push_back(c);

                    if (!foundEndOfHeaders) {
                        if (data.find(endOfHeaders) != std::string::npos) {
                            foundEndOfHeaders = true;

                            // Obtener la longitud del contenido del cuerpo de la solicitud
                            std::size_t contentLengthPos = data.find(contentLengthHeader);
                            if (contentLengthPos != std::string::npos) {
                                contentLengthPos += contentLengthHeader.length();
                                std::string contentLengthStr = cut(data.substr(contentLengthPos), "\r\n");
                                contentLength = std::stoi(contentLengthStr);
                            }
                        }
                    }

                    // Verificar si se ha leído el contenido completo
                    if (foundEndOfHeaders && data.length() - data.find(endOfHeaders) - endOfHeaders.length() >= contentLength) {
                        break;
                    }
                }

                if (bytesRead <= 0) {
                    // El cliente cerró la conexión o ocurrió un error, cerrar el socket del cliente
                    close(clientSocket);

                    // Remover el socket del conjunto de descriptores de sockets
                    FD_CLR(clientSocket, &readSockets);

                    std::cout << "Cliente desconectado. Socket: " << clientSocket << std::endl;
                    continue;
                }

                std::string dataresponse = data;
                std::cout << "Solicitud recibida:" << std::endl;
                std::cout << data << std::endl;

                // Extraer el contenido binario de la solicitud
                binary = findBinary(data, "\r\n\r\n");

                std::ofstream outputFile("archivo.txt", std::ios::binary);
                outputFile.write(binary.data(), binary.size());
                outputFile.close();

                // Verificar si el archivo se transfirió correctamente
                std::ifstream inputFile("archivo.txt", std::ios::binary);
                bool fileTransferSuccessful = inputFile.good();
                inputFile.close();

                std::string response;
                if (fileTransferSuccessful) {
                    response = generateResponse(dataresponse);
                } else {
                    response = "HTTP/1.1 500 Internal Server Error\r\n";
                    response += "Content-Type: text/plain\r\n";
                    response += "Content-Length: 21\r\n";
                    response += "\r\n";
                    response += "Error en la transferencia";
                }

                // Enviar la respuesta al cliente
                send(clientSocket, response.c_str(), response.length(), 0);

                // Limpiar los datos
                data.clear();

                std::cout << "Respuesta enviada:" << std::endl;
                std::cout << response << std::endl;
            }
        }
    }

    // Cerrar el socket del servidor
    close(serverSocket);

    return 0;
}

