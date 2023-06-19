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

std::string extractRequestPath(const std::string& data) {
    std::size_t startPos = data.find("GET ") + 4;
    std::size_t endPos = data.find(" ", startPos);
    if (startPos != std::string::npos && endPos != std::string::npos) {
        return data.substr(startPos, endPos - startPos);
    }
    return "";
}

std::string getFileNameFromPath(const std::string& path) {
    std::size_t slashPos = path.find_last_of("/");
    if (slashPos != std::string::npos) {
        return path.substr(slashPos + 1);
    }
    return path;
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
    std::string fileBoundaryEnd = "\r\n--" + boundary + "--\r\n";
    std::size_t fileStartPos = data.find(fileBoundaryStart);
    std::size_t fileEndPos = data.rfind(fileBoundaryEnd);

    if (fileStartPos != std::string::npos && fileEndPos != std::string::npos) {
        fileStartPos += fileBoundaryStart.length();
        std::string fileContent = data.substr(fileStartPos, fileEndPos - fileStartPos);
        
        std::ofstream outputFile("uploaded_file.txt", std::ios::binary);
        if (outputFile) {
            outputFile.write(fileContent.c_str(), fileContent.length());
            outputFile.close();
            return true;
        }
    }

    return false;
}

std::string loadStaticContent(const std::string& fileName) {
    std::ifstream file(fileName, std::ios::binary | std::ios::ate);
    if (file) {
        std::ifstream::pos_type fileSize = file.tellg();
        file.seekg(0, std::ios::beg);

        std::string fileContent(fileSize, ' ');
        file.read(&fileContent[0], fileSize);

        file.close();
        return fileContent;
    }
    return "";
}

void sendResponse(int clientSocket, const std::string& response) {
    ssize_t bytesSent = send(clientSocket, response.c_str(), response.length(), 0);
    if (bytesSent < 0) {
        std::cerr << "Error al enviar la respuesta" << std::endl;
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

    std::string response;

    std::string requestPath = extractRequestPath(data);
    std::string fileName = getFileNameFromPath(requestPath);

    if (!fileName.empty()) {
        std::string fileContent = loadStaticContent(fileName);
        if (!fileContent.empty()) {
            response = "HTTP/1.1 200 OK\r\n"
                       "Content-Type: image/jpeg\r\n"
                       "Content-Length: " + std::to_string(fileContent.length()) + "\r\n"
                       "\r\n" + fileContent;
        } else {
            response = "HTTP/1.1 404 Not Found\r\n"
                       "Content-Type: text/html\r\n"
                       "Content-Length: 0\r\n"
                       "\r\n";
        }
    } else {
        std::string staticContent = loadStaticContent("static.html");
        response = "HTTP/1.1 200 OK\r\n"
                   "Content-Type: text/html\r\n"
                   "Content-Length: " + std::to_string(staticContent.length()) + "\r\n"
                   "\r\n" + staticContent;
    }

    sendResponse(clientSocket, response);

    close(clientSocket);
}

int main() {
    int serverSocket = createServerSocket();
    bindServerSocket(serverSocket, 8080);
    listen(serverSocket, 5);

    std::cout << "Servidor a la escucha en el puerto 8080..." << std::endl;

    while (true) {
        int clientSocket = acceptClientConnection(serverSocket);
        handleClientRequest(clientSocket);
    }

    close(serverSocket);

    return 0;
}

