#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fstream>
#include <cstring>
#include <list>
#include <map>
#include <sstream>
#include <cctype>

class Server {
public:
    explicit Server(int port);
    Server();
    ~Server();
    void start();

private:
    int 			createServerSocket();
    void 			bindServerSocket(int serverSocket, int port);
    int 			acceptClientConnection(int serverSocket);
    std::string 	receiveData(int clientSocket, int contentLength);
    std::string 	extractBoundary(const std::string& data);
    bool 			saveFileContent(const std::string& data, const std::string& boundary);
    void 			sendResponse(int clientSocket);
    void 			handleClientRequest(int clientSocket);
	void 			extractBoundary(const std::string &key, const std::map<std::string, std::list<std::string> > &keyValuePairs, std::string& boundary);
	void 			extractValues(const std::string &key, const std::map<std::string, std::list<std::string> > &keyValuePairs, std::string& value);
	void 			extractFilename(const std::string& key, const std::map<std::string, std::list<std::string> >& keyValuePairs, std::string& filename);
	void 			printValueForKey(const std::string& key, const std::map<std::string, std::list<std::string> >& keyValuePairs);
	void 			parseRequest(const std::string& request, std::map<std::string, std::list<std::string> >& keyValuePairs);
	std::string		findBinary(const std::string& cadena, const std::string& separador);
	void			printMap(std::map<std::string, std::list<std::string> > &keyValuePairs);
	bool 			isASCII(const std::string& str);


    int port;
    int serverSocket;
	std::string fileName;
};

#endif

