#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fstream>
#include <cstring>
#include <map>
#include <sstream>
#include <list>

void extractBoundary(const std::string& key, const std::map<std::string, std::list<std::string>>& keyValuePairs, std::string& boundary) 
{
    std::map<std::string, std::list<std::string>>::const_iterator it = keyValuePairs.find(key);
    if (it != keyValuePairs.end()) {
        const std::list<std::string>& values = it->second;
        std::string boundaryPrefix = "boundary=";

        for (const std::string& value : values) {
            std::size_t prefixPos = value.find(boundaryPrefix);
            if (prefixPos != std::string::npos) {
                boundary = value.substr(prefixPos + boundaryPrefix.length());
                break;
            }
        }

        if (!boundary.empty()) {
            std::cout << "Boundary: " << boundary << std::endl;
        } else {
            std::cout << "Boundary not found in " << key << std::endl;
        }
    } else {
        std::cout << "Key not found: " << key << std::endl;
    }
}



void extractValues(const std::string& key, const std::map<std::string, std::list<std::string> >& keyValuePairs, std::string& value) {
    std::map<std::string, std::list<std::string> >::const_iterator it = keyValuePairs.find(key);
    if (it != keyValuePairs.end()) {
        const std::list<std::string>& values = it->second;
        
        if (!values.empty()) {
            value = values.front();
            std::cout << key << ": " << value << std::endl;
        } else {
            std::cout << "No value found for " << key << std::endl;
        }
    } else {
        std::cout << "Key not found: " << key << std::endl;
    }
}


void extractFilename(const std::string& key, const std::map<std::string, std::list<std::string> >& keyValuePairs, std::string& filename) {
    std::map<std::string, std::list<std::string> >::const_iterator it = keyValuePairs.find(key);
    if (it != keyValuePairs.end()) {
        const std::list<std::string>& values = it->second;
        std::string filenamePrefix = "filename=\"";
        std::string filenameSuffix = "\"";

        std::list<std::string>::const_iterator listIt;
        for (listIt = values.begin(); listIt != values.end(); ++listIt) {
            std::size_t prefixPos = listIt->find(filenamePrefix);
            if (prefixPos != std::string::npos) {
                std::size_t suffixPos = listIt->find(filenameSuffix, prefixPos + filenamePrefix.length());
                if (suffixPos != std::string::npos) {
                    filename = listIt->substr(prefixPos + filenamePrefix.length(), suffixPos - (prefixPos + filenamePrefix.length()));
                    break;
                }
            }
        }

        if (!filename.empty()) {
            std::cout << "Filename: " << filename << std::endl;
        } else {
            std::cout << "Filename not found in " << key << std::endl;
        }
    } else {
        std::cout << "Key not found: " << key << std::endl;
    }
}



void printValueForKey(const std::string& key, const std::map<std::string, std::list<std::string> >& keyValuePairs) {
    std::map<std::string, std::list<std::string> >::const_iterator it = keyValuePairs.find(key);
    if (it != keyValuePairs.end()) {
        const std::list<std::string>& values = it->second;

        std::cout << key << ": ";
        std::list<std::string>::const_iterator listIt;
        for (listIt = values.begin(); listIt != values.end(); ++listIt) {
            std::cout << *listIt << std::endl;
        }
        std::cout << std::endl;
    } else {
        std::cout << "Key not found: " << key << std::endl;
    }
}


std::string findBinary(const std::string& cadena, const std::string& separador) {
    std::string::size_type pos = cadena.find(separador);
    std::string::size_type lastPos = std::string::npos;

    while (pos != std::string::npos) {
        lastPos = pos;
        pos = cadena.find(separador, pos + 1);
    }

    if (lastPos != std::string::npos) {
        return cadena.substr(lastPos + separador.length());
    }

    return "";
}

std::string cut(const std::string& cadena, const std::string& separador) {
    std::size_t pos = cadena.find(separador);
    if (pos != std::string::npos) {
        return cadena.substr(0, pos);
    }
    return cadena;
}

void parseRequest(const std::string& request, std::map<std::string, std::list<std::string> >& keyValuePairs) 
{
    std::istringstream iss(request);
    std::string line;

    // Leer línea por línea
    while (std::getline(iss, line)) {
        // Ignorar las líneas en blanco
        if (line.empty()) {
            continue;
        }

        // Buscar el separador entre clave y valor
        std::size_t separatorPos = line.find(':');
        if (separatorPos != std::string::npos) {
            // Obtener la clave y el valor
            std::string key = line.substr(0, separatorPos);
            std::string value = line.substr(separatorPos + 1);

            // Eliminar espacios en blanco alrededor de la clave y el valor
            key.erase(0, key.find_first_not_of(' '));
            key.erase(key.find_last_not_of(' ') + 1);
            value.erase(0, value.find_first_not_of(' '));
            value.erase(value.find_last_not_of(' ') + 1);

            // Almacenar el par clave: valor en el mapa
            keyValuePairs[key].push_back(value);
        }
    }
}

int main() {
    // Crear el socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    std::string data;
    std::string binary;
    std::string temp;
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
    int i = 0;
    while ((bytesRead = recv(clientSocket, &c, 1, 0)) > 0) {
        data.push_back(c);
    }
    std::cout << data << std::endl;

    binary = findBinary(data, "\r\n\r\n");

    // std::cout << binary << std::endl;

    std::ofstream outputFile("archivo.jpg", std::ios::binary);
    outputFile.write(binary.data(), binary.size());
    outputFile.close();

    std::map<std::string, std::list<std::string> > keyValuePairs;
    parseRequest(data, keyValuePairs);

    // Mostrar el contenido clave: valor
	
	std::string filename;
	std::string boundary;	
    printValueForKey("Content-Type", keyValuePairs);
    extractFilename("Content-Disposition", keyValuePairs, filename);
    extractBoundary("Content-Type", keyValuePairs, boundary);
//    extractValues("Content-Type", keyValuePairs, boundary);
	



    // Cerrar los sockets
    close(clientSocket);

    close(serverSocket);

    return 0;
}

