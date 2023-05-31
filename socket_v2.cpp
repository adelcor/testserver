#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <vector>
#include <fstream>

const int MAX_CLIENTS = 10;  // Número máximo de clientes

int main() {
    // Crear el socket del servidor
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Error al crear el socket." << std::endl;
        return 1;
    }

    // Configurar la dirección y el puerto para enlazar el socket
    sockaddr_in serverAddress;
	memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);  // Puerto 8080
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    // Enlazar el socket
    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        std::cerr << "Error al enlazar el socket." << std::endl;
        close(serverSocket);
        return 1;
    }

    // Escuchar el socket
    if (listen(serverSocket, 10) == -1) {
        std::cerr << "Error al escuchar el socket." << std::endl;
        close(serverSocket);
        return 1;
    }

    std::cout << "Esperando conexiones entrantes..." << std::endl;

    // Crear el conjunto de descriptores de archivo
    fd_set allSockets;
    FD_ZERO(&allSockets);
    FD_SET(serverSocket, &allSockets);

    int maxDescriptor = serverSocket;  // El descriptor más alto (por ahora, es el del servidor)

    std::vector<int> clientSockets;  // Vector para almacenar los descriptores de los sockets de los clientes

    while (true) {
        // Copiar el conjunto de descriptores de archivo
        fd_set readSockets = allSockets;

        // Llamar a la función select para esperar actividad en los sockets
        int activity = select(maxDescriptor + 1, &readSockets, nullptr, nullptr, nullptr);
        if (activity == -1) {
            std::cerr << "Error en select." << std::endl;
            close(serverSocket);
            return 1;
        }

        // Verificar si hay actividad en el socket del servidor
        if (FD_ISSET(serverSocket, &readSockets)) {
            // Aceptar una nueva conexión
            sockaddr_in clientAddress;
			memset(&serverAddress, 0, sizeof(serverAddress));
            socklen_t clientAddressLength = sizeof(clientAddress);
            int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLength);
            if (clientSocket == -1) {
                std::cerr << "Error al aceptar la conexión." << std::endl;
                close(serverSocket);
                return 1;
            }

            std::cout << "Se ha establecido una nueva conexión." << std::endl;

            // Agregar el nuevo descriptor de socket al conjunto de descriptores de archivo
            FD_SET(clientSocket, &allSockets);

            // Actualizar el descriptor más alto si es necesario
            if (clientSocket > maxDescriptor) {
                maxDescriptor = clientSocket;
            }

            // Agregar el nuevo descriptor de socket al vector de sockets de clientes
            clientSockets.push_back(clientSocket);
        }

        // Verificar actividad en los sockets de los clientes
        for (auto it = clientSockets.begin(); it != clientSockets.end(); ) {
            int clientSocket = *it;

            if (FD_ISSET(clientSocket, &readSockets)) {
                // Leer datos del cliente
                char buffer[1024];
                ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
                if (bytesRead == -1) {
                    std::cerr << "Error al leer los datos del cliente." << std::endl;
                    close(clientSocket);
                    it = clientSockets.erase(it);
                    continue;
                }

                if (bytesRead == 0) {
                    // El cliente cerró la conexión
                    std::cout << "Conexión cerrada por el cliente." << std::endl;
                    close(clientSocket);
                    it = clientSockets.erase(it);
                    continue;
                }

                // Procesar los datos recibidos
                std::cout << "Datos recibidos del cliente:" << std::endl;
                std::cout.write(buffer, bytesRead);
                std::cout << std::endl;

		std::string request(buffer, bytesRead);
		if(request.find("Content-Disposition: form-data;") != std::string::npos)
		{
			std::cout << "HOLA" << std::endl;
			continue;


		}

	
                // Enviar una respuesta al cliente
                const char* response = "HTTP/1.1 200 OK\r\nContent-Length: 12\r\n\r\nHello World!";
                ssize_t bytesSent = send(clientSocket, response, strlen(response), 0);
                if (bytesSent == -1) {
                    std::cerr << "Error al enviar la respuesta al cliente." << std::endl;
                    close(clientSocket);
                    it = clientSockets.erase(it);
                    continue;
                }
            }

            ++it;
        }
    }

    // Cerrar todos los sockets de los clientes
    for (int clientSocket : clientSockets) {
        close(clientSocket);
    }

    // Cerrar el socket del servidor
    close(serverSocket);

    return 0;
}

