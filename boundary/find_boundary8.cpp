#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fstream>
#include <cstring>


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

    // Leer y mostrar las peticiones recibidas
    char c;
    ssize_t bytesRead;
    while ((bytesRead = recv(clientSocket, &c, 1, 0)) > 0) {
        data.push_back(c);
    }
    std::cout << data << std::endl;

    // Extraer el valor de "boundary" de los encabezados
    std::string boundary;
    std::size_t boundaryPos = data.find("boundary=");
    if (boundaryPos != std::string::npos) {
        std::size_t startPos = boundaryPos + 9;
        std::size_t endPos = data.find("\r\n", startPos);
        if (endPos != std::string::npos) {
            boundary = data.substr(startPos, endPos - startPos);
        }
    }

    std::cout << "Valor de boundary: " << boundary << std::endl;

    // Buscar la posición inicial y final del contenido del archivo
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

            // Guardar el contenido en un archivo
            std::ofstream outputFile("archivo_recibido.txt");
            outputFile << fileContent;
            outputFile.close();

            std::cout << "Contenido del archivo guardado exitosamente." << std::endl;
        } else {
            std::cerr << "No se pudo encontrar el contenido del archivo." << std::endl;
        }
    } else {
        std::cerr << "No se pudo encontrar el archivo en la solicitud." << std::endl;
    }

    // Enviar respuesta al cliente
    std::string response = "HTTP/1.1 200 OK\r\n"
                           "Content-Type: text/plain\r\n"
                           "Content-Length: 9\r\n"
                           "\r\n"
                           "Perfecto!";
    ssize_t bytesSent = send(clientSocket, response.c_str(), response.length(), 0);
    if (bytesSent != response.length()) {
        std::cerr << "Error al enviar la respuesta al cliente." << std::endl;
    }

    // Cerrar los sockets
    close(clientSocket);
    close(serverSocket);

    return 0;
}

