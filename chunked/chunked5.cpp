#include <iostream>
#include <sstream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <fstream>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

const int PORT = 8080;
const int BUFFER_SIZE = 1024;

std::string parseChunkedRequest(const char* request) {
  std::string data;

  // Skip the request headers
  const char* start = strstr(request, "\r\n\r\n");
  if (start == NULL) {
    return data; // Invalid request
  }
  start += 4;

  // Process each chunk
  while (true) {
    char* endPtr;
    unsigned long chunkSize = strtoul(start, &endPtr, 16);
    if (chunkSize == 0) {
      break; // Last chunk
    }

    start = endPtr + 2; // Skip chunk size and CRLF

    // Append chunk data to the result
    data.append(start, chunkSize);

    start += chunkSize + 2; // Skip chunk data and CRLF
  }

  return data;
}

std::string createResponse() {
  std::string response = "HTTP/1.1 200 OK\r\n";
  response += "Content-Type: text/plain\r\n\r\n";
  response += "Hello, world!";

  return response;
}

void saveFile(const std::string& filename, const std::string& content) {
  std::ofstream file(filename.c_str(), std::ios::out | std::ios::binary);
  if (file) {
    file.write(content.data(), content.size());
    file.close();
    std::cout << "File saved: " << filename << std::endl;
  } else {
    std::cerr << "Error saving file: " << filename << std::endl;
  }
}

int main() {
  int serverSocket, clientSocket;
  struct sockaddr_in serverAddress, clientAddress;

  // Create socket
  serverSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (serverSocket < 0) {
    std::cerr << "Error opening socket." << std::endl;
    return 1;
  }

  // Set server address
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(PORT);
  serverAddress.sin_addr.s_addr = INADDR_ANY;

  // Bind the socket
  if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
    std::cerr << "Error binding socket." << std::endl;
    return 1;
  }

  // Listen for incoming connections
  listen(serverSocket, 5);
  std::cout << "Server listening on port " << PORT << std::endl;

  while (true) {
    socklen_t clientAddressLength = sizeof(clientAddress);

    // Accept client connection
    clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLength);
    if (clientSocket < 0) {
      std::cerr << "Error accepting client connection." << std::endl;
      continue;
    }

    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));

    // Read client request
    ssize_t bytesRead = read(clientSocket, buffer, sizeof(buffer) - 1);
    if (bytesRead < 0) {
      std::cerr << "Error reading from socket." << std::endl;
      close(clientSocket);
      continue;
    }

    std::cout << "Received request:\n" << buffer << std::endl;

    // Check if "Expect: 100-continue" header is present
    if (strstr(buffer, "Expect: 100-continue") != NULL) {
      // Send "100 Continue" response
      const char* continueResponse = "HTTP/1.1 100 Continue\r\n\r\n";
      ssize_t bytesSent = write(clientSocket, continueResponse, strlen(continueResponse));
      if (bytesSent < 0) {
        std::cerr << "Error writing to socket." << std::endl;
        close(clientSocket);
        continue;
      }
    }

    // Parse the chunked request
    std::string requestData = parseChunkedRequest(buffer);
    std::cout << "Received data:\n" << requestData << std::endl;

    // Save the received file
    saveFile("perro.jpg", requestData);

    // Create response
    std::string response = createResponse();

    // Send response
    ssize_t bytesSent = write(clientSocket, response.c_str(), response.length());
    if (bytesSent < 0) {
      std::cerr << "Error writing to socket." << std::endl;
    }

    // Close client connection
    close(clientSocket);
  }

  // Close server socket
  close(serverSocket);

  return 0;
}

