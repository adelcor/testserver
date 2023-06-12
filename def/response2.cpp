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
    int clientSocket;
    
    std::cout << "Servidor a la escucha en el puerto 8080..." << std::endl;
    
    // Aceptar una conexión entrante
    clientSocket = accept(serverSocket, nullptr, nullptr);
    if (clientSocket < 0) {
        std::cerr << "Error al aceptar la conexión" << std::endl;
        close(serverSocket);
        return 1;
    }
    
    // Leer los primeros datos de la solicitud
    char c;
    ssize_t bytesRead;
    int foundCount = 0;
    while ((bytesRead = recv(clientSocket, &c, 1, 0)) > 0) {
        data.push_back(c);
//	foundCount++;
//	if(foundCount > 1000000)
//		break;
        
        // Verificar si se ha encontrado la cadena "\r\n\r\n"
        if (data.find("\r\n\r\n") != std::string::npos) {
            foundCount++;
            
            // Detener el bucle después de la segunda ocurrencia
            if (foundCount == 3) {
                break;
            }
        }
    }
    
    std::cout << data << std::endl;
    
    binary = findBinary(data, "\r\n\r\n");
    
    std::cout << binary << std::endl;
    
    std::ofstream outputFile("archivo.jpg", std::ios::binary);
    outputFile.write(binary.data(), binary.size());
    outputFile.close();
    
    // Enviar una respuesta al cliente
    std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 12\r\n\r\nHello, client";
    send(clientSocket, response.c_str(), response.size(), 0);
    
    // Cerrar los sockets
    close(clientSocket);
    close(serverSocket);
    
    return 0;
}

