#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sstream>

int main() {
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddress, clientAddress;

    // Crear el socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Error al crear el socket." << std::endl;
        return 1;
    }

    // Configurar la dirección del servidor
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(8080);

    // Enlazar el socket al puerto
    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1) {
        std::cerr << "Error al enlazar el socket al puerto." << std::endl;
        close(serverSocket);
        return 1;
    }

    // Escuchar conexiones entrantes
    if (listen(serverSocket, 5) == -1) {
        std::cerr << "Error al escuchar conexiones entrantes." << std::endl;
        close(serverSocket);
        return 1;
    }

    std::cout << "Servidor en ejecución. Esperando conexiones..." << std::endl;

    while (true) {
        socklen_t clientAddressLength = sizeof(clientAddress);

        // Aceptar una conexión entrante
        clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientAddressLength);
        if (clientSocket == -1) {
            std::cerr << "Error al aceptar la conexión entrante." << std::endl;
            close(serverSocket);
            return 1;
        }

        // Leer la solicitud del cliente
        ssize_t bytesRead;
        std::string data;

        while (true) {
            char c;
            bytesRead = recv(clientSocket, &c, 1, 0);
            if (bytesRead == -1) {
                std::cerr << "Error al leer la solicitud del cliente." << std::endl;
                close(clientSocket);
                close(serverSocket);
                return 1;
            } else if (bytesRead == 0) {
                // El cliente cerró la conexión
                break;
            }

//            std::cout << c;
            data.push_back(c);

            // Verificar si se alcanzó el final de la solicitud
            
        }

		
		std::string search = "\r\n\r\n";
		std::string output = data;
		size_t pos = data.find(search);
		while(pos != std::string::npos)
		{
			output.insert(pos + search.length(), "SALTO AQUI");
    		pos = output.find(search, pos + search.length() + 1);
		}
		std::cout << output << std::endl;

 // Imprimir la solicitud en pantalla
//        std::cout << "Solicitud recibida: " << data << std::endl;

        // Cerrar la conexión con el cliente
        close(clientSocket);
    }

    // Cerrar el socket del servidor
    close(serverSocket);

    return 0;
}

