#include "server.hpp"

Server::Server()
{
}

Server::~Server()
{
}

int Server::createSocket(void)
{
	this->serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(this->serverSocket == -1)
	{
		std::cerr << "Error al crear el socket." << std::endl;
		return(1);
	}
	return(0);
}

void Server::configureSocket(void)
{
	std::memset(&(this->serverAddress), 0, sizeof(this->serverAddress));
	this->serverAddress.sin_family = AF_INET;
	this->serverAddress.sin_port = htons(8081);
	this->serverAddress.sin_addr.s_addr = INADDR_ANY;
}

int Server::linkSocket(void)
{
	if(bind(this->serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1)
	{
		std::cerr << "Error al enlazar el socket." << std::endl;
		close(this->serverSocket);
		return(1);
	}
	return(0);
}

int Server::listenSocket(void)
{
	if(listen(this->serverSocket, 10) == -1)
	{
		std::cerr << "Error al escuchar el socket." << std::endl;
		close(this->serverSocket);
		return(1);
	}
	std::cout << "Esperando conexiones entrantes..." << std::endl;
	return(0);
}

void Server::createFileDescriptors(void)
{
	FD_ZERO(&this->allSockets);
	FD_SET(this->serverSocket, &this->allSockets);
	this->maxDescriptor = this->serverSocket;
}

int Server::callSelect(void)
{
	this->activity = select(this->maxDescriptor + 1, &this->readSockets, NULL, NULL, NULL);
	if(this->activity == -1)
	{
		std::cerr << "Error en select." << std::endl;
		close(serverSocket);
		return(1);
	}
	return(0);
}

int Server::checkActivity(void)
{
	if(FD_ISSET(this->serverSocket, &this->readSockets))
	{
		std::memset(&(this->clientAddress), 0, sizeof(this->clientAddress));
		this->clientAddressLength = sizeof(this->clientAddress);
		this->clientSocket = accept(this->serverSocket, (struct sockaddr*)&this->clientAddress, &this->clientAddressLength);
		if(this->clientSocket == -1)
		{
			std::cerr << "Error al aceptar la conexion." << std::endl;
			close(this->serverSocket);
			return(1);
		}
		std::cout << "Se ha establecido una nueva conexion" << std::endl;
		handleFileDescriptors();
	}
	return(0);

}

void Server::handleFileDescriptors(void)
{
	FD_SET(this->clientSocket, &this->allSockets);
	if(this->clientSocket > this->maxDescriptor)
		this->maxDescriptor = this->clientSocket;
	this->clientSockets.push_back(clientSocket);
}

void Server::writeData(void)
{
	std::cout << "Datos recibidos del cliente:" << std::endl;
	std::cout.write(this->buffer, bytesRead);
	std::cout << std::endl;
}

void Server::checkClients(void)
{
	for(std::vector<int>::iterator it = this->clientSockets.begin(); it != this->clientSockets.end(); )
	{
		this->clientSocket = *it;
		if(FD_ISSET(this->clientSocket, &this->readSockets))
		{
			
			this->bytesRead = recv(this->clientSocket, this->buffer, sizeof(this->buffer), 0);

			if(this->bytesRead == -1)
			{
				std::cerr << "Error al leer los datos del cliente." << std::endl;
				close(this->clientSocket);
				it = this->clientSockets.erase(it);
				continue;
			}

			if(this->bytesRead == 0)
			{
				std::cout << "Conexion cerrada por el cliente." << std::endl;
				close(this->clientSocket);
				it = this->clientSockets.erase(it);
				continue;
			}

			writeData();
			std::string request(this->buffer, this->bytesRead);
			if(request.find("Content-Disposition: form-data;") != std::string::npos)
			{
				std::cout << "Intento de subir un archivo detectado." << std::endl;
				continue;
			}

			const char* response = "HTTP/1.1 200 OK\r\nContent-Length: 12\r\n\r\nHello World!";
			this->bytesSent = send(this->clientSocket, response, strlen(response), 0);
			if(this->bytesSent == -1)
			{
				std::cerr << "Error al enviar la respuesta al cliente." << std::endl;
				close(this->clientSocket);
				it = this->clientSockets.erase(it);
				continue;
			}
		}
		++it;
	}
}






int Server::run(void)
{
	while(true)
	{
		this->readSockets = this->allSockets;
		if(callSelect())
			return(1);
		if(checkActivity())
			return(1);
		checkClients();
	}
	std::vector<int>::iterator it;
	for(it = this->clientSockets.begin(); it != clientSockets.end(); ++it)
	{
		this->clientSocket = *it;
		close(this->clientSocket);
	}
	close (this->serverSocket);
}
















































