#include <cstdlib>
#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

int main() {
    // Create a socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Failed to create socket." << std::endl;
        return 1;
    }

    // Bind the socket to a specific IP address and port
    struct sockaddr_in serverAddress;
	    memset(&serverAddress, 0, sizeof(serverAddress));  // Initialize serverAddress with memset

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(80);
    serverAddress.sin_addr.s_addr = inet_addr("192.168.0.100");  // Replace with your desired IP address

    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1) {
        std::cerr << "Failed to bind socket." << std::endl;
        close(serverSocket);
        return 1;
    }

    // Start listening for incoming connections
    if (listen(serverSocket, 5) == -1) {
        std::cerr << "Failed to listen on socket." << std::endl;
        close(serverSocket);
        return 1;
    }

    std::cout << "Server is running on IP address 192.168.0.100" << std::endl;

    // Accept and handle incoming connections
    while (true) {
        int clientSocket = accept(serverSocket, nullptr, nullptr);
        // Handle the client request here...
    }

    // Close the server socket
    close(serverSocket);

    return 0;
}

