#include "server.hpp"

std::string parseChunkedRequest(const char* request) {
  std::string data;

  // Skip the request headers
  const char* start = strstr(request, "\r\n\r\n");
  if (start == NULL) {
    return data; // Invalid request
  }
  start += 4;

  // Process each chunk
  while (true) {
    char* endPtr;
    unsigned long chunkSize = strtoul(start, &endPtr, 16);
    if (chunkSize == 0) {
      break; // Last chunk
    }

    start = endPtr + 2; // Skip chunk size and CRLF

    // Append chunk data to the result
    data.append(start, chunkSize);

    start += chunkSize + 2; // Skip chunk data and CRLF
  }

  return data;
}


void saveFile(const std::string& filename, const std::string& content) {
  std::ofstream file(filename.c_str(), std::ios::out | std::ios::binary);
  if (file) {
    file.write(content.data(), content.size());
    file.close();
    std::cout << "File saved: " << filename << std::endl;
  } else {
    std::cerr << "Error saving file: " << filename << std::endl;
  }
}


std::string removeCarriageReturn(const std::string& value)
{
    std::string trimmedValue;
    for (char c : value)
    {
        if (c != '\r')
        {
            trimmedValue += c;
        }
    }
    return trimmedValue;
}

std::string cut(const std::string& cadena, const std::string& separador) 
{
    std::size_t pos = cadena.find(separador);
    if (pos != std::string::npos) 
    {
        return cadena.substr(0, pos);
    }
    return cadena;
}

void Server::printMap(KeyValueMap &keyValuePairs)
{
	MapIterator it;
	ListIterator listIt;
	
	for (it = keyValuePairs.begin(); it != keyValuePairs.end(); ++it) 
	{
		std::cout << "Clave: " << it->first << std::endl;
		std::cout << "Valores:" << std::endl;
		
		for (listIt = it->second.begin(); listIt != it->second.end(); ++listIt) 
		{
			std::cout << "- " << *listIt << std::endl;
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


std::string Server::getDeletedFilename(const std::string &requestData)
{
	std::string filename;
	std::size_t startPos = requestData.find("DELETE /") + 8;
	std::size_t endPos = requestData.find(" HTTP/1.1\r\n");
	if (endPos != std::string::npos)
	{
		filename = requestData.substr(startPos, endPos - startPos);
	}
	return filename;
}

std::string Server::getRequestedFilename(const std::string& requestData) 
{
	std::string filename;
	std::size_t startPos = requestData.find("GET /") + 5;
	std::size_t endPos = requestData.find(" HTTP/1.1\r\n");
	if (endPos != std::string::npos) 
	{
		filename = requestData.substr(startPos, endPos - startPos);
	}
	return filename;
}

std::string Server::loadStaticContent(const std::string& filename) 
{
	std::ifstream inputFile(filename, std::ios::binary);
	if (!inputFile) 
	{
		std::cerr << "Error al abrir el archivo: " << filename << std::endl;
		return "";
	}
	
	std::string content((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());
	inputFile.close();
	return content;
}

std::string Server::loadStatic(void) 
{
	std::ifstream inputFile("index.html", std::ios::binary);
	if (!inputFile) 
	{
		std::cerr << "Error al abrir el archivo: " << std::endl;
		return "";
	}
	std::string content((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());
	inputFile.close();
	return content;
}

void Server::start() 
{
	serverSocket = createServerSocket();
	bindServerSocket(serverSocket, port);
	
	if (listen(serverSocket, 1) < 0) 
	{
		std::cerr << "Error al escuchar en el socket" << std::endl;
		close(serverSocket);
		exit(1);
	}
	
	std::cout << "Servidor a la escucha en el puerto " << port << "..." << std::endl;
	
	while (true) 
    {
		int clientSocket = acceptClientConnection(serverSocket);
		handleClientRequest(clientSocket);
	}
	
	close(serverSocket);
}

int Server::createServerSocket() 
{
	int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket < 0) 
	{
		std::cerr << "Error al crear el socket" << std::endl;
		exit(1);
	}
	return serverSocket;
}

void Server::bindServerSocket(int serverSocket, int port) 
{
	struct sockaddr_in serverAddress;
	memset(&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = INADDR_ANY;
	serverAddress.sin_port = htons(port);
	
	if (bind(serverSocket, reinterpret_cast<struct sockaddr*>(&serverAddress), sizeof(serverAddress)) < 0) 
	{
		std::cerr << "Error al vincular el socket" << std::endl;
		close(serverSocket);
		exit(1);
	}
}

int Server::acceptClientConnection(int serverSocket) 
{
	int clientSocket = accept(serverSocket, nullptr, nullptr);
	if (clientSocket < 0) 
	{
		std::cerr << "Error al aceptar la conexión" << std::endl;
		close(serverSocket);
		exit(1);
	}
	return clientSocket;
}


std::string Server::extractBoundary(const std::string& data) 
{
	std::string boundary;
	std::size_t boundaryPos = data.find("boundary=");
	if (boundaryPos != std::string::npos) 
	{
		std::size_t startPos = boundaryPos + 9;
		std::size_t endPos = data.find("\r\n", startPos);
		if (endPos != std::string::npos) 
		{
			boundary = data.substr(startPos, endPos - startPos);
		}
	}
	return boundary;
}

bool Server::saveFileContent(const std::string& data, const std::string& boundary) 
{
	std::string fileBoundaryStart = "--" + boundary + "\r\n";
	std::string fileBoundaryEnd = "--" + boundary + "--";
	std::size_t fileStartPos = data.find(fileBoundaryStart);
	std::size_t fileEndPos = data.find(fileBoundaryEnd, fileStartPos);
	
	if (fileStartPos != std::string::npos && fileEndPos != std::string::npos) 
	{
		std::string fileContent;
		std::size_t fileContentStartPos = data.find("\r\n\r\n", fileStartPos);
		if (fileContentStartPos != std::string::npos) 
		{
			fileContentStartPos += 4;
			fileContent = data.substr(fileContentStartPos, fileEndPos - fileContentStartPos - 2);
			
			std::ofstream outputFile(this->fileName);
			outputFile << fileContent;
			outputFile.close();
			std::cout << "Contenido del archivo guardado exitosamente." << std::endl;
            return true;
        } 
		else 
		{
			std::cerr << "No se pudo encontrar el contenido del archivo." << std::endl;
		}
	} 
	
	else 
	{
		std::cerr << "No se pudo encontrar el archivo en la solicitud." << std::endl;
    }
	return false;
}

void Server::sendResponse(int clientSocket, const std::string& response) 
{
	ssize_t bytesSent = send(clientSocket, response.c_str(), response.length(), 0);
	if (bytesSent != response.length()) 
	{
		std::cerr << "Error al enviar la respuesta al cliente." << std::endl;
	}
}

void Server::handleClientRequest(int clientSocket) 
{
	std::string data;
	char c;
	ssize_t bytesRead;
	int contentLength = 0;
	bool foundEndOfHeaders = false;
	std::string endOfHeaders = "\r\n\r\n";
	std::string contentLengthHeader = "Content-Length: ";
	
	while ((bytesRead = recv(clientSocket, &c, 1, 0)) > 0) 
	{
		data.push_back(c);
		if (!foundEndOfHeaders) 
		{
			if (data.find(endOfHeaders) != std::string::npos) 
			{
				foundEndOfHeaders = true;
				std::size_t contentLengthPos = data.find(contentLengthHeader);
				if (contentLengthPos != std::string::npos) 
				{
					contentLengthPos += contentLengthHeader.length();
					std::string contentLengthStr = cut(data.substr(contentLengthPos), "\r\n");
					contentLength = std::stoi(contentLengthStr);
				}
			}
		}
		
		if (foundEndOfHeaders && data.length() - data.find(endOfHeaders) - endOfHeaders.length() >= contentLength) 
		{
			break;
        }
	}
	std::cout << data << std::endl;
	std::string requestMethod = cut(data, " ");
	KeyValueMap keyValuePairs;
	parseRequest(data, keyValuePairs);
//	printValueForKey("Content-Type", keyValuePairs);


//	printValueForKey("Transfer-Encoding", keyValuePairs);	


	extractFilename("Content-Disposition", keyValuePairs);

	std::cout << "------------------------------------\n";
	std::cout << "----------MAP-----------------------\n";
	printMap(keyValuePairs);
	std::cout << "------------------------------------\n";
	
	if(requestMethod == "GET")
    {
	    handleGetRequest(clientSocket, data);
    }
    else if(requestMethod == "POST")
    {
	    handlePostRequest(clientSocket, data, keyValuePairs);
    }
	else if(requestMethod == "DELETE")
	{
		handleDeleteRequest(clientSocket, data);
	}
	
	else
	{
	    std::cerr << "Metodo de solicitud no valido." << std::endl;
    }

	close(clientSocket);
	this->fileName.clear();
}


void    Server::handleDeleteRequest(int clientSocket, const std::string &requestData)
{
	std::cout << "----SOLICITUD DELETE-----\n";
	std::string deleted = getDeletedFilename(requestData);
	std::cout << "Deleted es: " << deleted << std::endl;
	if(remove(deleted.c_str()) != 0)
		std::cout << "El archivo no pudo ser eliminado\n";
	else
		std::cout << "Archivo eliminado correctamente\n";

	std::string staticContent = loadStatic();
	std::string resp = "HTTP/1.1 200 OK\r\n"
					   "Content-Type: text/html\r\n"
					   "Content-Length: " + std::to_string(staticContent.length()) + "\r\n"
					   "\r\n" + staticContent;
	sendResponse(clientSocket, resp);
}

void  Server::handleGetRequest(int clientSocket, const std::string& requestData) 
{
    std::string response;
    std::string fileName = getRequestedFilename(requestData);

    if (fileName != "index.html" && fileName != " /" && !fileName.empty()) 
	{
		std::string fileContent = loadStaticContent(fileName);
		if (!fileContent.empty()) 
		{
			response = "HTTP/1.1 200 OK\r\n"
					   "Content-Type: image/jpeg\r\n"
					   "Content-Length: " + std::to_string(fileContent.length()) + "\r\n"
					   "\r\n" + fileContent;
		} 
		
		else 
		{
			response = "HTTP/1.1 404 Not Found\r\n"
                       "Content-Type: text/html\r\n"
                       "Content-Length: 0\r\n"
                       "\r\n";
		}
	} 
	
	else 
	{
		std::string staticContent = loadStatic();
		response = "HTTP/1.1 200 OK\r\n"
                   "Content-Type: text/html\r\n"
                   "Content-Length: " + std::to_string(staticContent.length()) + "\r\n"
                   "\r\n" + staticContent;
	}
	
	sendResponse(clientSocket, response);
}

void Server::handleChunked(int clientSocket, const std::string &requestData, const KeyValueMap &keyValuePairs)
{
	int bytesRead;
	std::string data = requestData;
	char buffer[1024];
	std::cout << "ESTOY AQUI\n";
	if(requestData.find("Expect: 100-continue") != std::string::npos)
		std::cout << "RESPUESTA CONTINUE ENVIADA\n";
	
//	const char* continueResponse = "100 Continue\r\n\r\n";
	std::string response =
        "100 Continue\r\n"
        "Date: Mon, 27 Jul 2009 12:28:53 GMT\r\n"
        "Server: Apache/2.2.14 (Win32)\r\n"
        "Last-Modified: Wed, 22 Jul 2009 19:15:56 GMT\r\n"
        "Content-Length: 88\r\n"
        "Content-Type: text/html\r\n";
	ssize_t bytesSent = send(clientSocket, response.c_str(), response.length(), 0);
	if (bytesSent < 0)
	{
		std::cerr << "Error writing to socket." << std::endl;
		close(clientSocket);
	
	}

	while(true)
	{
		memset(buffer, 0, sizeof(buffer));
		bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
		if (bytesRead <= 0) 
		{
			break; // Error or end of request
		}
		data.append(buffer, bytesRead);
	}
	std::cout << "data despues de la respuesta\n";
	std::cout << data << std::endl;
	std::string requestBody = parseChunkedRequest(data.c_str());
	std::cout << "Received data:\n" << requestBody << std::endl;
	std::cout << "filename es \n";
	std::cout << this->fileName << std::endl;
	saveFile(this->fileName, requestBody);








}


void Server::handlePostRequest(int clientSocket, const std::string& requestData, const KeyValueMap &keyValuePairs) 
{
	if(isInMap("Transfer-Encoding", "chunked\r", keyValuePairs))
	{
		std::cout << "PUES VA A SER QUE ES UNA PETICION CHUNKED\n";
		handleChunked(clientSocket, requestData, keyValuePairs);
	}

	std::string boundary = extractBoundary(requestData);
	
	if (!boundary.empty()) 
	{
		saveFileContent(requestData, boundary);
	}
	
	std::string staticContent = loadStatic();
	std::string response = "HTTP/1.1 200 OK\r\n"
                           "Content-Type: text/html\r\n"
                           "Content-Length: " + std::to_string(staticContent.length()) + "\r\n"
                           "\r\n" + staticContent;
	
	sendResponse(clientSocket, response);
	
}


void Server::extractValues(const std::string &key, const KeyValueMap &keyValuePairs, std::string& value) 
{
	MapIterator it = keyValuePairs.find(key);
	if (it != keyValuePairs.end()) 
	{
		const std::list<std::string>& values = it->second;
		if (!values.empty()) 
		{
			value = values.front();
			std::cout << key << ": " << value << std::endl;
		} 
		
		else 
		{
			std::cout << "No value found for " << key << std::endl;
		}
	} 
	else 
	{
		std::cout << "Key not found: " << key << std::endl;
	}
}

void Server::extractFilename(const std::string& key, const KeyValueMap &keyValuePairs) 
{
	MapIterator it = keyValuePairs.find(key);
	if (it != keyValuePairs.end()) 
	{
		const std::list<std::string>& values = it->second;
		std::string filenamePrefix = "filename=\"";
		std::string filenameSuffix = "\"";
		
		std::list<std::string>::const_iterator listIt;
		for (listIt = values.begin(); listIt != values.end(); ++listIt) 
		{
			std::size_t prefixPos = listIt->find(filenamePrefix);
			if (prefixPos != std::string::npos) 
			{
				std::size_t suffixPos = listIt->find(filenameSuffix, prefixPos + filenamePrefix.length());
				if (suffixPos != std::string::npos) 
				{
					this->fileName = listIt->substr(prefixPos + filenamePrefix.length(), suffixPos - (prefixPos + filenamePrefix.length()));
					break;
				}
			}
		}
		
		if (!this->fileName.empty()) 
		{
			std::cout << "Filename: " << this->fileName << std::endl;
		} 
		else 
		{
			std::cout << "Filename not found in " << key << std::endl;
		}
	} 
	
	else 
	{
		std::cout << "Key not found: " << key << std::endl;
    }
}

void Server::printValueForKey(const std::string& key, const KeyValueMap &keyValuePairs) 
{
	MapIterator it = keyValuePairs.find(key);
	if (it != keyValuePairs.end()) 
	{
		const std::list<std::string>& values = it->second;
		std::cout << key << ": ";
        ListIterator listIt;
        for (listIt = values.begin(); listIt != values.end(); ++listIt) 
		{
			std::cout << *listIt << std::endl;
		}
		std::cout << std::endl;
	} 
	
	else 
	{
		std::cout << "Key not found: " << key << std::endl;
    }
}



bool Server::isInMap(const std::string& key, const std::string &value, const KeyValueMap &keyValuePairs)
{
	MapIterator it = keyValuePairs.find(key);
	if (it != keyValuePairs.end())
	{
		const std::list<std::string>& values = it->second;
		ListIterator listIt;
		for (listIt = values.begin(); listIt != values.end(); ++listIt)
		{
			if(*listIt == value)
				return true;
			else
				return false;
		}
	}
	
	std::cout << "Key not found: " << key << std::endl;
	
	return false;
}




bool Server::isASCII(const std::string& str)
{
	for (std::string::const_iterator it = str.begin(); it != str.end(); ++it)
	{
		if (static_cast<unsigned char>(*it) > 127)
		{
			return false;
		}
	}
	return true;
}




void Server::parseRequest(const std::string &request, KeyValueMap &keyValuePairs)
{
    std::istringstream iss(request);
    std::string line;
	
	while (std::getline(iss, line)) 
	{
		if (line.empty() || !isASCII(line)) 
		{
			continue;
		}
		
		std::size_t separatorPos = line.find(':');
		
		if (separatorPos != std::string::npos) 
		{
			std::string key = line.substr(0, separatorPos);
			std::string value = line.substr(separatorPos + 1);
            key.erase(0, key.find_first_not_of(' '));
            key.erase(key.find_last_not_of(' ') + 1);
            value.erase(0, value.find_first_not_of(' '));
            value.erase(value.find_last_not_of(' ') + 1);
            keyValuePairs[key].push_back(value);
        }
    }
}












