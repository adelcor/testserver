#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

const int PORT = 8080;
const std::string WEB_ROOT = "/Users/adel-cor/testserver/static";

std::string getMimeType(const std::string& filename) {
    if (filename.find(".html") != std::string::npos)
        return "text/html";
    else if (filename.find(".css") != std::string::npos)
        return "text/css";
    else if (filename.find(".js") != std::string::npos)
        return "application/javascript";
    else if (filename.find(".jpg") != std::string::npos || filename.find(".jpeg") != std::string::npos)
        return "image/jpeg";
    else if (filename.find(".png") != std::string::npos)
        return "image/png";
    else if (filename.find(".gif") != std::string::npos)
        return "image/gif";
    else if (filename.find(".ico") != std::string::npos)
        return "image/x-icon";
    else
        return "text/plain";
}

std::string readFile(const std::string& filename) {
    std::ifstream file(filename.c_str(), std::ios::in | std::ios::binary);
    if (file) {
        std::ostringstream contents;
        contents << file.rdbuf();
        file.close();
        return contents.str();
    }
    return "";
}

void handleRequest(int clientSocket) {
    char buffer[1024];
    int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesRead > 0) {
        std::istringstream request(buffer);
        std::string requestLine;
        getline(request, requestLine);
        std::istringstream requestLineStream(requestLine);
        std::string method, path, httpVersion;
        requestLineStream >> method >> path >> httpVersion;
		std::cout << "PATH: " << path << std::endl;

        // Remove leading slash from the path
        path.erase(0, 1);

        // Generate the full file path
        std::string fullPath = WEB_ROOT + "/" + (path.empty() ? "test.html" : path);

		std::cout << "FULLPATH: " << fullPath << std::endl;

        // Open the requested file
        std::string response;
        std::string fileContents = readFile(fullPath);
        if (!fileContents.empty()) {
            std::string mimeType = getMimeType(path);
            response = "HTTP/1.1 200 OK\r\n"
                       "Content-Type: " + mimeType + "\r\n"
                       "Content-Length: " + std::to_string(fileContents.length()) + "\r\n"
                       "\r\n" + fileContents;
        } else {
            response = "HTTP/1.1 404 Not Found\r\n"
                       "Content-Type: text/html\r\n"
                       "Content-Length: 9\r\n"
                       "\r\n"
                       "Not found";
        }

        // Send the response to the client
        send(clientSocket, response.c_str(), response.length(), 0);
    }
    close(clientSocket);
}

int main() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        std::cerr << "Error creating server socket." << std::endl;
        return 1;
    }

    sockaddr_in serverAddress;
	memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(PORT);

    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        std::cerr << "Error binding server socket." << std::endl;
        return 1;
    }

    if (listen(serverSocket, 5) < 0) {
        std::cerr << "Error listening on server socket." << std::endl;
        return 1;
    }

    std::cout << "Server running on port " << PORT << std::endl;

    while (true) {
        sockaddr_in clientAddress;
		memset(&clientAddress, 0, sizeof(clientAddress));
        socklen_t clientAddressLength = sizeof(clientAddress);
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLength);
        if (clientSocket < 0) {
            std::cerr << "Error accepting client connection." << std::endl;
            return 1;
        }

        handleRequest(clientSocket);
    }

    close(serverSocket);
    return 0;
}

