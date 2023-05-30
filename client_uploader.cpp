#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fstream>

#define SERVER_IP "127.0.0.1"
#define PORT 8080
#define BUFFER_SIZE 4096

int main() {
    int clientSocket;
    struct sockaddr_in serverAddress;
    char buffer[BUFFER_SIZE];

    // Crear el socket del cliente
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        std::cerr << "Error al crear el socket del cliente" << std::endl;
        return 1;
    }

    // Configurar la dirección del servidor
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    if (inet_pton(AF_INET, SERVER_IP, &(serverAddress.sin_addr)) <= 0) {
        std::cerr << "Dirección IP inválida" << std::endl;
        return 1;
    }

    // Conectar con el servidor
    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        std::cerr << "Error al conectar con el servidor" << std::endl;
        return 1;
    }

    std::cout << "Conectado al servidor" << std::endl;

    // Abrir el archivo para lectura
    std::ifstream file("archivo.txt", std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error al abrir el archivo para lectura" << std::endl;
        close(clientSocket);
        return 1;
    }

    // Obtener el tamaño del archivo
// Obtener el tamaño del archivo
	file.seekg(0, std::ios::end);
	std::streampos fileSize = file.tellg();
	file.seekg(0, std::ios::beg);

// fileSize contiene el tamaño del archivo en bytes

    // Enviar metadatos del archivo
    if (send(clientSocket, &fileSize, sizeof(fileSize), 0) < 0) {
        std::cerr << "Error al enviar los metadatos del archivo" << std::endl;
        close(clientSocket);
        file.close();
        return 1;
    }

    // Enviar el contenido del archivo
    while (!file.eof()) {
        file.read(buffer, BUFFER_SIZE);
        int bytesRead = file.gcount();
        if (send(clientSocket, buffer, bytesRead, 0) < 0) {
            std::cerr << "Error al enviar el contenido del archivo" << std::endl;
            close(clientSocket);
            file.close();
            return 1;
        }
    }

    file.close();
    std::cout << "Archivo enviado correctamente" << std::endl;

    // Cerrar la conexión con el servidor
    close(clientSocket);

    return 0;
}

