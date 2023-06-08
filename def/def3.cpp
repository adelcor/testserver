#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fstream>

std::string findBinary(const std::string& cadena, const std::string& separador) {
    std::string::size_type pos = cadena.find(separador);
    if (pos != std::string::npos) {
        return cadena.substr(pos + separador.length());
    }
    return "";
}

int main() {
    // Crear el socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    std::string data;

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
    if (bind(serverSocket, reinterpret_cast<struct sockaddr*>(&serverAddress), sizeof(serverAddress)) < 0) {
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

    // Leer y mostrar las peticiones recibidas
    char buffer[1024]; // Buffer de lectura
    ssize_t bytesRead;
    while ((bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0) {
        data.append(buffer, bytesRead);
    }

    // Encontrar el inicio y el final del contenido del archivo
    std::string::size_type startPos = data.find("\r\n\r\n");
    std::string::size_type endPos = data.rfind("\r\n--");
    if (startPos != std::string::npos && endPos != std::string::npos && endPos > startPos) {
        startPos += 4; // Saltar el delimitador inicial
        std::string binary = data.substr(startPos, endPos - startPos);

        // Buscar y eliminar las líneas de encabezado adicionales
        std::string::size_type headerStart = binary.find("\r\n\r\n");
        if (headerStart != std::string::npos) {
            binary = binary.substr(headerStart + 4);
        }

        std::cout << binary << std::endl;

        // Guardar el contenido binario en un archivo
        std::ofstream outputFile("archivo.json", std::ios::binary);
        outputFile.write(binary.data(), binary.size());
        outputFile.close();
    }

    // Cerrar los sockets
    close(clientSocket);
    close(serverSocket);

    return 0;
}

