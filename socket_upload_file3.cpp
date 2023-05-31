#include <iostream>
#include <fstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>


#define PORT 8080
#define BUFFER_SIZE 4096

int main() {
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddress, clientAddress;
    socklen_t clientAddressLength = sizeof(clientAddress);
    char buffer[BUFFER_SIZE];
	char buffer2[BUFFER_SIZE];

    // Crear el socket del servidor
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Error al crear el socket del servidor" << std::endl;
        return 1;
    }

    // Configurar la dirección del servidor
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(PORT);

    // Vincular el socket a la dirección del servidor
    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        std::cerr << "Error al vincular el socket del servidor" << std::endl;
        return 1;
    }

    // Escuchar conexiones entrantes
    if (listen(serverSocket, 5) < 0) {
        std::cerr << "Error al escuchar conexiones entrantes" << std::endl;
        return 1;
    }

    std::cout << "Servidor escuchando en el puerto " << PORT << std::endl;

    while (true) {
        // Aceptar una nueva conexión entrante
        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLength);
        if (clientSocket < 0) {
            std::cerr << "Error al aceptar la conexión entrante" << std::endl;
            continue;
        }

        std::cout << "Cliente conectado: " << inet_ntoa(clientAddress.sin_addr) << std::endl;

        // Recibir el archivo enviado por el cliente
        std::ofstream file("archivo_recibido.txt", std::ios::binary);
        if (!file) {
            std::cerr << "Error al abrir el archivo para escritura" << std::endl;
            close(clientSocket);
            continue;
        }

        ssize_t bytesRead;
        while ((bytesRead = recv(clientSocket, buffer2, BUFFER_SIZE, 0)) > 0) {
            file.write(buffer2, bytesRead);
        }

        file.close();

        std::cout << "Archivo recibido y guardado correctamente" << std::endl;

        // Cerrar la conexión con el cliente
        close(clientSocket);
    }

    // Cerrar el socket del servidor
    close(serverSocket);

    return 0;
}

