#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define PORT 8080

void handle_request(int client_socket) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);

    // Leer la petición del cliente
    read(client_socket, buffer, BUFFER_SIZE - 1);
    std::cout << "Petición recibida:\n" << buffer << std::endl;

    // Extraer el nombre del archivo del encabezado de la petición
    std::string request(buffer);
	std::cout << "buffer es: " << buffer << std::endl;
    std::string::size_type start = request.find("filename=\"") + 10;
    std::string::size_type end = request.find("\"", start);
    std::string filename = request.substr(start, end - start);
	std::cout << "filename es: " << filename << std::endl;

    // Abrir el archivo para escritura
    std::ofstream file(filename.c_str(), std::ios::binary);
    if (!file.is_open()) {
        std::cout << "Error al abrir el archivo" << std::endl;
        return;
    }

    // Leer y escribir los datos del archivo desde el cuerpo de la solicitud PUT
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_read = read(client_socket, buffer, BUFFER_SIZE);
        if (bytes_read <= 0) {
            break;
        }
        file.write(buffer, bytes_read);
    }

    // Cerrar el archivo y enviar una respuesta al cliente
    file.close();
    const char *response = "Archivo recibido correctamente";
    write(client_socket, response, strlen(response));

    close(client_socket);
}

int main() {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        std::cerr << "Error al crear el socket" << std::endl;
        return 1;
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        std::cerr << "Error al enlazar el socket" << std::endl;
        return 1;
    }

    if (listen(server_socket, 5) < 0) {
        std::cerr << "Error al poner en escucha el socket" << std::endl;
        return 1;
    }

    std::cout << "Servidor iniciado. Esperando conexiones..." << std::endl;

    while (true) {
        struct sockaddr_in client_address;
        socklen_t client_address_length = sizeof(client_address);

        int client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_length);
        if (client_socket < 0) {
            std::cerr << "Error al aceptar la conexión" << std::endl;
            continue;
        }

        handle_request(client_socket);
    }

    close(server_socket);

    return 0;
}

