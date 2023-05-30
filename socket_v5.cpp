#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <unistd.h>

const int BUFFER_SIZE = 1024;

std::string readFile(const std::string& filename) {
    std::ifstream file(filename.c_str(), std::ios::binary);
    if (!file) {
        return "";
    }

    std::ostringstream oss;
    oss << file.rdbuf();
    return oss.str();
}

void handleRequest(int clientSocket) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);

    int bytesRead = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
    if (bytesRead < 0) {
        std::cerr << "Error reading from socket." << std::endl;
        return;
    }

    std::istringstream iss(buffer);
    std::string requestType, requestPath;
    iss >> requestType >> requestPath;

    // Handle GET request
    if (requestType == "GET") {
        std::string filePath = "." + requestPath;
        std::string fileContent = readFile(filePath);

        std::string response;
        if (!fileContent.empty()) {
            response = "HTTP/1.1 200 OK\r\n"
                       "Content-Length: " + std::to_string(fileContent.length()) + "\r\n"
                       "Connection: close\r\n"
                       "\r\n" + fileContent;
        } else {
            response = "HTTP/1.1 404 Not Found\r\n"
                       "Connection: close\r\n"
                       "\r\n";
        }

        send(clientSocket, response.c_str(), response.length(), 0);
    }
    // Handle file upload
    else if (requestType == "POST") {
        std::string line;
        while (std::getline(iss, line) && !line.empty()) {
            if (line.find("Content-Length:") != std::string::npos) {
                std::istringstream contentLengthStream(line);
                std::string dummy;
                int contentLength;
                contentLengthStream >> dummy >> contentLength;

                char* fileContent = new char[contentLength];
                memset(fileContent, 0, contentLength);

                recv(clientSocket, fileContent, contentLength, 0);

                std::ofstream file("uploaded_file.txt", std::ios::binary);
                file.write(fileContent, contentLength);
                file.close();

                delete[] fileContent;

                break;
            }
        }

        std::string response = "HTTP/1.1 200 OK\r\n"
                               "Connection: close\r\n"
                               "\r\n";
        send(clientSocket, response.c_str(), response.length(), 0);
    }

    close(clientSocket);
}

int main() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        std::cerr << "Error creating socket." << std::endl;
        return 1;
    }

    struct sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr*) &serverAddress, sizeof(serverAddress)) < 0) {
        std::cerr << "Error binding socket." << std::endl;
        return 1;
    }

    if (listen(serverSocket, 5) < 0) {
        std::cerr << "Error listening on socket." << std::endl;
        return 1;
    }

    std::cout << "Server listening on port 8080..." << std::endl;

    while (true) {
        struct sockaddr_in clientAddress{};
        socklen_t clientAddressLength = sizeof(clientAddress);

        int clientSocket = accept(serverSocket, (struct sockaddr*) &clientAddress, &clientAddressLength);
        if (clientSocket < 0) {
            std::cerr << "Error accepting connection." << std::endl;
            continue;
        }

        handleRequest(clientSocket);
    }

    close(serverSocket);

    return 0;
}

