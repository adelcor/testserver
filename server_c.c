#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024
#define PORT 8080

/*void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);

    // Leer la petición del cliente
    read(client_socket, buffer, BUFFER_SIZE - 1);
    printf("Petición recibida:\n%s\n", buffer);

    // Extraer el nombre del archivo del encabezado de la petición
    char *filename_start = strstr(buffer, "filename=\"") + 10;
    char *filename_end = strchr(filename_start, '\"');
    size_t filename_length = filename_end - filename_start;
    char filename[filename_length + 1];
    strncpy(filename, filename_start, filename_length);
    filename[filename_length] = '\0';

    // Abrir el archivo para escritura
    FILE *file = fopen(filename, "wb");
    if (file == NULL) {
        printf("Error al abrir el archivo\n");
        return;
    }

    // Leer y escribir los datos del archivo
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_read = read(client_socket, buffer, BUFFER_SIZE);
        if (bytes_read <= 0) {
            break;
        }
        fwrite(buffer, 1, bytes_read, file);
    }

    // Cerrar el archivo y enviar una respuesta al cliente
    fclose(file);
    const char *response = "Archivo recibido correctamente";
    write(client_socket, response, strlen(response));

    close(client_socket);
}*/

/*void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);

    // Leer la petición del cliente
    read(client_socket, buffer, BUFFER_SIZE - 1);
    printf("Petición recibida:\n%s\n", buffer);

    // Extraer el nombre del archivo del encabezado de la petición
    char *filename_start = strstr(buffer, "filename=\"") + 10;
    char *filename_end = strchr(filename_start, '\"');
    size_t filename_length = filename_end - filename_start;
    char filename[filename_length + 1];
    strncpy(filename, filename_start, filename_length);
    filename[filename_length] = '\0';

    // Abrir el archivo para escritura
    FILE *file = fopen(filename, "wb");
    if (file == NULL) {
        printf("Error al abrir el archivo\n");
        return;
    }

    // Leer y escribir los datos del archivo desde el cuerpo de la solicitud PUT
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_read = read(client_socket, buffer, BUFFER_SIZE);
        if (bytes_read <= 0) {
            break;
        }
        fwrite(buffer, 1, bytes_read, file);
    }

    // Cerrar el archivo y enviar una respuesta al cliente
    fclose(file);
    const char *response = "Archivo recibido correctamente";
    write(client_socket, response, strlen(response));

    close(client_socket);
}*/

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);

    // Leer la petición del cliente
    read(client_socket, buffer, BUFFER_SIZE - 1);
    printf("Petición recibida:\n%s\n", buffer);

    // Extraer el nombre del archivo del encabezado de la petición
    char *filename_start = strstr(buffer, "filename=\"") + 10;
    char *filename_end = strchr(filename_start, '\"');
    size_t filename_length = filename_end - filename_start;
    char filename[filename_length + 1];
    strncpy(filename, filename_start, filename_length);
    filename[filename_length] = '\0';

    // Abrir el archivo para escritura
    FILE *file = fopen(filename, "wb");
    if (file == NULL) {
        printf("Error al abrir el archivo\n");
        return;
    }

    // Leer y escribir los datos del archivo desde el cuerpo de la solicitud PUT
    int content_length = 0;
    char *content_length_start = strstr(buffer, "Content-Length:");
    if (content_length_start != NULL) {
        sscanf(content_length_start, "Content-Length: %d", &content_length);
    }

    if (content_length > 0) {
        int bytes_read = 0;
        int bytes_remaining = content_length;

        while (bytes_remaining > 0) {
            memset(buffer, 0, BUFFER_SIZE);
            int bytes_to_read = BUFFER_SIZE < bytes_remaining ? BUFFER_SIZE : bytes_remaining;
            bytes_read = read(client_socket, buffer, bytes_to_read);
            if (bytes_read <= 0) {
                break;
            }
            fwrite(buffer, 1, bytes_read, file);
            bytes_remaining -= bytes_read;
        }
    }

    // Cerrar el archivo y enviar una respuesta al cliente
    fclose(file);
    const char *response = "Archivo recibido correctamente";
    write(client_socket, response, strlen(response));

    close(client_socket);
}


int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    socklen_t client_address_length = sizeof(client_address);

    // Crear el socket del servidor
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        printf("Error al crear el socket\n");
        return 1;
    }

    // Configurar el servidor
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    // Vincular el socket a la dirección y puerto
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        printf("Error al vincular el socket\n");
        return 1;
    }

    // Escuchar las conexiones entrantes
    if (listen(server_socket, 5) == -1) {
        printf("Error al escuchar las conexiones\n");
        return 1;
    }

    printf("Servidor iniciado. Esperando conexiones...\n");

    while (1) {
        // Aceptar una nueva conexión de cliente
        client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_length);
        if (client_socket == -1) {
            printf("Error al aceptar la conexión\n");
            return 1;
        }

        // Manejar la conexión en un proceso hijo
        if (fork() == 0) {
            // Proceso hijo
            close(server_socket);
            handle_client(client_socket);
            exit(0);
        } else {
            // Proceso padre
            close(client_socket);
        }
    }

    // Cerrar el socket del servidor
    close(server_socket);

    return 0;
}

