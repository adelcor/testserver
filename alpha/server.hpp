#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fstream>
#include <cstring>

class Server {
public:
    explicit Server(int port);
    Server();
    ~Server();
    void start();

private:
    int createServerSocket();
    void bindServerSocket(int serverSocket, int port);
    int acceptClientConnection(int serverSocket);
    std::string receiveData(int clientSocket, int contentLength);
    std::string extractBoundary(const std::string& data);
    bool saveFileContent(const std::string& data, const std::string& boundary);
    void sendResponse(int clientSocket);
    void handleClientRequest(int clientSocket);

    int port;
    int serverSocket;
};

#endif

