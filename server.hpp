#ifndef SERVER_HPP
#define SERVER_HPP


#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <vector>
#include <fstream>

extern const int MAX_CLIENTS;


class Server
{
	private:
		int			serverSocket;
		int			maxDescriptor;
		int			activity;
		int			clientSocket;
		ssize_t			bytesRead;
		ssize_t			bytesSent;
		sockaddr_in		serverAddress;
		sockaddr_in		clientAddress;
		socklen_t		clientAddressLength;
		fd_set			allSockets;
		fd_set			readSockets;
		std::vector<int> 	clientSockets;
		char			buffer[1024];


	public:
		Server();
		~Server();
		int 	createSocket(void);
		void 	configureSocket(void);
		int 	linkSocket(void);
		int	listenSocket(void);
		void	createFileDescriptors(void);
		int	callSelect(void);
		int 	checkActivity(void);
		void	handleFileDescriptors(void);
		void	checkClients(void);
		void	writeData(void);
		int	run(void);
};

#endif



