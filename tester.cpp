#include <iostream>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

const int NUM_THREADS = 10; // Número de hilos a utilizar
const int NUM_REQUESTS_PER_THREAD = 1000; // Número de peticiones por hilo

void* sendRequests(void* arg) {
    const char* serverIP = "127.0.0.1"; // Dirección IP del servidor
    int serverPort = 8080; // Puerto del servidor

    // Crear el socket del cliente
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        std::cerr << "Error al crear el socket del cliente." << std::endl;
        pthread_exit(nullptr);
    }

    // Configurar la dirección y el puerto del servidor
    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(serverPort);
    inet_pton(AF_INET, serverIP, &(serverAddress.sin_addr));

    // Conectar al servidor
    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        std::cerr << "Error al conectar al servidor." << std::endl;
        close(clientSocket);
        pthread_exit(nullptr);
    }

    // Enviar peticiones al servidor
    const char* request = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n";
    char response[1024];

    for (int i = 0; i < NUM_REQUESTS_PER_THREAD; ++i) {
        if (send(clientSocket, request, strlen(request), 0) == -1) {
            std::cerr << "Error al enviar la petición al servidor." << std::endl;
            break;
        }

        if (recv(clientSocket, response, sizeof(response), 0) == -1) {
            std::cerr << "Error al recibir la respuesta del servidor." << std::endl;
            break;
        }
    }

    // Cerrar el socket del cliente
    close(clientSocket);

    pthread_exit(nullptr);
}

int main() {
    std::vector<pthread_t> threads(NUM_THREADS);

    // Crear los hilos
    for (int i = 0; i < NUM_THREADS; ++i) {
        if (pthread_create(&threads[i], nullptr, sendRequests, nullptr) != 0) {
            std::cerr << "Error al crear el hilo." << std::endl;
            return 1;
        }
    }

    // Esperar a que los hilos terminen
    for (int i = 0; i < NUM_THREADS; ++i) {
        pthread_join(threads[i], nullptr);
    }

    return 0;
}

