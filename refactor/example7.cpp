#include <iostream>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>

int main() {
    // Configurar socket
    int socket = 0; // Usamos el descriptor de archivo 0 para el socket

    // Configurar conjuntos de descriptores de archivo para select
    fd_set readfds, writefds;
    FD_ZERO(&readfds);
    FD_ZERO(&writefds);
    FD_SET(socket, &readfds);

    

    // Set the socket to non-blocking mode using fcntl
    int flags = fcntl(socket, F_GETFL, 0);
    fcntl(socket, F_SETFL, flags | O_NONBLOCK);

 

    // Esperar eventos en el socket
    while(true)
    {
	    //Configurar el timeout de select
		
	    timeval timeout{};
	    timeout.tv_sec = 10; // 10 segundos
	    timeout.tv_usec = 0;

	    std::cout << "Introduce la peticion\n\n";

	    int result = select(socket + 1, &readfds, &writefds, nullptr, &timeout);
	    if (result == -1)
	    {
		    std::cerr << "Error en select()." << std::endl;
		    return 1;
	    } 
	    else if (result == 0)
	    {
		    std::cout << "Timeout de select alcanzado." << std::endl;
		    return 0;
	    }

    // Verificar si el socket está listo para lectura

	    if (FD_ISSET(socket, &readfds))
	    {
		    char buffer[1024];
		    ssize_t bytesRead = read(socket, buffer, sizeof(buffer));
		    if (bytesRead == -1)
		    {
			    std::cerr << "Error al leer del socket." << std::endl;
			    return 1;
		    }
		    
		    else if (bytesRead == 0)
		    {
			    std::cout << "El socket se cerró." << std::endl;
			    return 0;
		    }

        // Procesar los datos leídos

        // Cambiar el socket al conjunto de descriptores de archivo de escritura
	
		    FD_CLR(socket, &readfds);
		    FD_SET(socket, &writefds);
    }

    // Verificar si el socket está listo para escritura
    
	    if (FD_ISSET(socket, &writefds))
	    {
		    std::string response = "\nRespuesta del servidor\n\n";
		    ssize_t bytesWritten = write(socket, response.c_str(), response.length());
		    if (bytesWritten == -1)
		    {
			    std::cerr << "Error al escribir en el socket." << std::endl;
			    return 1;
		    }

        // El mensaje se envió correctamente

        // Cambiar el socket al conjunto de descriptores de archivo de lectura
	
		    FD_CLR(socket, &writefds);
		    FD_SET(socket, &readfds);
	    }
    }

    return 0;
}

