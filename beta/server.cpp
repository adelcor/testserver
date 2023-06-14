#include "server.hpp"

std::string cut(const std::string& cadena, const std::string& separador) {
    std::size_t pos = cadena.find(separador);
    if (pos != std::string::npos) {
        return cadena.substr(0, pos);
    }
    return cadena;
}

void Server::printMap(std::map<std::string, std::list<std::string> > &keyValuePairs)
{
	for (const auto& pair : keyValuePairs) {
        std::cout << "Clave: " << pair.first << std::endl;
        std::cout << "Valores:" << std::endl;

        for (const auto& value : pair.second) {
            std::cout << "- " << value << std::endl;
        }

        std::cout << std::endl;
    }
}



Server::Server(int port) : port(port), serverSocket(0)
{
}

Server::~Server()
{
}



void Server::start() {
    serverSocket = createServerSocket();
    bindServerSocket(serverSocket, port);

    if (listen(serverSocket, 1) < 0) {
        std::cerr << "Error al escuchar en el socket" << std::endl;
        close(serverSocket);
        exit(1);
    }

    std::cout << "Servidor a la escucha en el puerto " << port << "..." << std::endl;

    while (true) {
        int clientSocket = acceptClientConnection(serverSocket);
        handleClientRequest(clientSocket);
    }

    close(serverSocket);
}

int Server::createServerSocket() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        std::cerr << "Error al crear el socket" << std::endl;
        exit(1);
    }
    return serverSocket;
}

void Server::bindServerSocket(int serverSocket, int port) {
    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddress.sin_port = htons(port);

    if (bind(serverSocket, reinterpret_cast<struct sockaddr*>(&serverAddress), sizeof(serverAddress)) < 0) {
        std::cerr << "Error al vincular el socket" << std::endl;
        close(serverSocket);
        exit(1);
    }
}

int Server::acceptClientConnection(int serverSocket) {
    int clientSocket = accept(serverSocket, nullptr, nullptr);
    if (clientSocket < 0) {
        std::cerr << "Error al aceptar la conexión" << std::endl;
        close(serverSocket);
        exit(1);
    }
    return clientSocket;
}

std::string Server::receiveData(int clientSocket, int contentLength) {
    std::string data;
    char c;
    ssize_t bytesRead;
    ssize_t totalBytesRead = 0;

    while ((bytesRead = recv(clientSocket, &c, 1, 0)) > 0) {
        data.push_back(c);
        totalBytesRead++;

        if (totalBytesRead >= contentLength)
            break;
    }

    return data;
}

std::string Server::extractBoundary(const std::string& data) {
    std::string boundary;
    std::size_t boundaryPos = data.find("boundary=");
    if (boundaryPos != std::string::npos) {
        std::size_t startPos = boundaryPos + 9;
        std::size_t endPos = data.find("\r\n", startPos);
        if (endPos != std::string::npos) {
            boundary = data.substr(startPos, endPos - startPos);
        }
    }
    return boundary;
}

bool Server::saveFileContent(const std::string& data, const std::string& boundary) {
    std::string fileBoundaryStart = "--" + boundary + "\r\n";
    std::string fileBoundaryEnd = "--" + boundary + "--";
    std::size_t fileStartPos = data.find(fileBoundaryStart);
    std::size_t fileEndPos = data.find(fileBoundaryEnd, fileStartPos);

    if (fileStartPos != std::string::npos && fileEndPos != std::string::npos) {
        std::string fileContent;
        std::size_t fileContentStartPos = data.find("\r\n\r\n", fileStartPos);
        if (fileContentStartPos != std::string::npos) {
            fileContentStartPos += 4;
            fileContent = data.substr(fileContentStartPos, fileEndPos - fileContentStartPos - 2);

            std::ofstream outputFile(this->fileName);
            outputFile << fileContent;
            outputFile.close();

            std::cout << "Contenido del archivo guardado exitosamente." << std::endl;
            return true;
        } else {
            std::cerr << "No se pudo encontrar el contenido del archivo." << std::endl;
        }
    } else {
        std::cerr << "No se pudo encontrar el archivo en la solicitud." << std::endl;
    }

    return false;
}

void Server::sendResponse(int clientSocket) {
    std::string response = "HTTP/1.1 200 OK\r\n"
                           "Content-Type: text/plain\r\n"
                           "Content-Length: 9\r\n"
                           "\r\n"
                           "Perfecto!";
    ssize_t bytesSent = send(clientSocket, response.c_str(), response.length(), 0);
    if (bytesSent != response.length()) {
        std::cerr << "Error al enviar la respuesta al cliente." << std::endl;
    }
}

void Server::handleClientRequest(int clientSocket) {
    std::string data;
    int contentLength = 0;
    bool foundEndOfHeaders = false;
    std::string endOfHeaders = "\r\n\r\n";
    std::string contentLengthHeader = "Content-Length: ";

    while (true) {
        char c;
        ssize_t bytesRead = recv(clientSocket, &c, 1, 0);
        if (bytesRead <= 0) {
            break;
        }

        data.push_back(c);

        if (!foundEndOfHeaders) {
            if (data.find(endOfHeaders) != std::string::npos) {
                foundEndOfHeaders = true;
                std::size_t contentLengthPos = data.find(contentLengthHeader);
                if (contentLengthPos != std::string::npos) {
                    contentLengthPos += contentLengthHeader.length();
                    std::string contentLengthStr = cut(data.substr(contentLengthPos), "\r\n");
                    contentLength = std::stoi(contentLengthStr);
                }
            }
        }

        if (foundEndOfHeaders && data.length() - data.find(endOfHeaders) - endOfHeaders.length() >= contentLength) {
            break;
        }
    }
//	std::cout << data << std::endl;
	std::map<std::string, std::list<std::string> > keyValuePairs;
	parseRequest(data, keyValuePairs);
	printValueForKey("Content-Type", keyValuePairs);
	extractFilename("Content-Disposition", keyValuePairs, this->fileName);
	printMap(keyValuePairs);



    std::string boundary = extractBoundary(data);
//    std::cout << "Valor de boundary: " << boundary << std::endl;

    bool fileSaved = saveFileContent(data, boundary);

    sendResponse(clientSocket);

    close(clientSocket);

    if (!fileSaved) {
        data.clear();
    }
}

void Server::extractBoundary(const std::string& key, const std::map<std::string, std::list<std::string> >& keyValuePairs, std::string& boundary)
{
    std::map<std::string, std::list<std::string> >::const_iterator it = keyValuePairs.find(key);
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
            std::cout << "Boundary2: " << boundary << std::endl;
        } else {
            std::cout << "Boundary not found in " << key << std::endl;
        }
    } else {
        std::cout << "Key not found: " << key << std::endl;
    }
}

void Server::extractValues(const std::string& key, const std::map<std::string, std::list<std::string> >& keyValuePairs, std::string& value) {
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

void Server::extractFilename(const std::string& key, const std::map<std::string, std::list<std::string> >& keyValuePairs, std::string& filename) {
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

void Server::printValueForKey(const std::string& key, const std::map<std::string, std::list<std::string> >& keyValuePairs) {
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

std::string Server::findBinary(const std::string& cadena, const std::string& separador) {
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

bool Server::isASCII(const std::string& str) {
    for (char c : str) {
        if (static_cast<unsigned char>(c) > 127) {
            return false;
        }
    }
    return true;
}

void Server::parseRequest(const std::string& request, std::map<std::string, std::list<std::string> >& keyValuePairs)
{
    std::istringstream iss(request);
    std::string line;

    // Leer línea por línea
    while (std::getline(iss, line)) {
        // Ignorar las líneas en blanco
        if (line.empty() || !isASCII(line)) {
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












