#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fstream>
#include <cstring>

std::string cut(const std::string& cadena, const std::string& separador) {
    std::size_t pos = cadena.find(separador);
    if (pos != std::string::npos) {
        return cadena.substr(0, pos);
    }
    return cadena;
}

int createServerSocket() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        std::cerr << "Error al crear el socket" << std::endl;
        exit(1);
    }
    return serverSocket;
}

void bindServerSocket(int serverSocket, int port) {
    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddress.sin_port = htons(port);

    if (bind(serverSocket, reinterpret_cast<struct sockaddr*>(&serverAddress), sizeof(serverAddress)) < 0) {
        std::cerr << "Error al vincular el socket" << std::endl;
        close(serverSocket);
        exit(1);
    }
}

int acceptClientConnection(int serverSocket) {
    int clientSocket = accept(serverSocket, nullptr, nullptr);
    if (clientSocket < 0) {
        std::cerr << "Error al aceptar la conexión" << std::endl;
        close(serverSocket);
        exit(1);
    }
    return clientSocket;
}

std::string receiveData(int clientSocket, int contentLength) {
    std::string data;
    char c;
    ssize_t bytesRead;
    ssize_t totalBytesRead = 0;

    while ((bytesRead = recv(clientSocket, &c, 1, 0)) > 0) {
        data.push_back(c);
        totalBytesRead += bytesRead;

        if (totalBytesRead >= contentLength) {
            break;
        }
    }

    return data;
}

std::string extractBoundary(const std::string& data) {
    std::string boundary;
    std::size_t boundaryPos = data.find("boundary=");
    if (boundaryPos != std::string::npos) {
        std::size_t startPos = boundaryPos + 9;
        std::size_t endPos = data.find("\r\n", startPos);
        if (endPos != std::string::npos) {
            boundary = data.substr(startPos, endPos - startPos);
        }
    }
    return boundary;
}

bool saveFileContent(const std::string& data, const std::string& boundary) {
    std::string fileBoundaryStart = "--" + boundary + "\r\n";
    std::string fileBoundaryEnd = "--" + boundary + "--";
    std::size_t fileStartPos = data.find(fileBoundaryStart);
    std::size_t fileEndPos = data.find(fileBoundaryEnd, fileStartPos);

    if (fileStartPos != std::string::npos && fileEndPos != std::string::npos) {
        std::string fileContent;
        std::size_t fileContentStartPos = data.find("\r\n\r\n", fileStartPos);
        if (fileContentStartPos != std::string::npos) {
            fileContentStartPos += 4;
            fileContent = data.substr(fileContentStartPos, fileEndPos - fileContentStartPos - 2);

            std::ofstream outputFile("archivo_recibido.png");
            outputFile << fileContent;
            outputFile.close();

            std::cout << "Contenido del archivo guardado exitosamente." << std::endl;
            return true;
        } else {
            std::cerr << "No se pudo encontrar el contenido del archivo." << std::endl;
        }
    } else {
        std::cerr << "No se pudo encontrar el archivo en la solicitud." << std::endl;
    }

    return false;
}

std::string loadStaticContent() {
    std::ifstream inputFile("static.html");
    std::string content((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());
    inputFile.close();
    return content;
}

void sendResponse(int clientSocket) {
    std::string staticContent = loadStaticContent();

    std::string response = "HTTP/1.1 200 OK\r\n"
                           "Content-Type: text/html\r\n"
                           "Content-Length: " + std::to_string(staticContent.length()) + "\r\n"
                           "\r\n" + staticContent;

    ssize_t bytesSent = send(clientSocket, response.c_str(), response.length(), 0);
    if (bytesSent != response.length()) {
        std::cerr << "Error al enviar la respuesta al cliente." << std::endl;
    }
}

void handleClientRequest(int clientSocket) {
    std::string data;
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
                std::size_t contentLengthPos = data.find(contentLengthHeader);
                if (contentLengthPos != std::string::npos) {
                    contentLengthPos += contentLengthHeader.length();
                    std::string contentLengthStr = cut(data.substr(contentLengthPos), "\r\n");
                    contentLength = std::stoi(contentLengthStr);
                }
            }
        }
        if (foundEndOfHeaders && data.length() - data.find(endOfHeaders) - endOfHeaders.length() >= contentLength) {
            break;
        }
    }

    std::cout << data << std::endl;

    std::string boundary = extractBoundary(data);
    std::cout << "Valor de boundary: " << boundary << std::endl;

    if (!boundary.empty()) {
        saveFileContent(data, boundary);
    }

    sendResponse(clientSocket);

    close(clientSocket);
}

int main() {
    int serverSocket = createServerSocket();
    bindServerSocket(serverSocket, 8080);

    if (listen(serverSocket, 1) < 0) {
        std::cerr << "Error al escuchar en el socket" << std::endl;
        close(serverSocket);
        exit(1);
    }

    std::cout << "Servidor a la escucha en el puerto 8080..." << std::endl;

    while (true) {
        int clientSocket = acceptClientConnection(serverSocket);
        handleClientRequest(clientSocket);
    }

    close(serverSocket);

    return 0;
}

