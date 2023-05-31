#include <iostream>
#include <fstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

int main() {
    // Crear el socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    
    // Configurar la dirección y el puerto
    struct sockaddr_in serverAddress;
	memset(&serverAddress , 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(12345);
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    
    // Enlazar el socket a la dirección y el puerto
    bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    
    // Poner el socket en modo de escucha
    listen(serverSocket, 1);
    
    // Esperar la conexión entrante
    int clientSocket = accept(serverSocket, nullptr, nullptr);
    
    // Ahora estás listo para recibir datos del archivo a través del socket
    std::ofstream outputFile("archivo_recibido.txt", std::ios::binary);
    char buffer[1024];
    int bytesRead;
    
    while ((bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0) {
        outputFile.write(buffer, bytesRead);
    }
    
    outputFile.close();
    
    std::cout << "Archivo recibido y guardado como 'archivo_recibido.txt'." << std::endl;
    
    // Cerrar los sockets
    close(clientSocket);
    close(serverSocket);
    
    return 0;
}

