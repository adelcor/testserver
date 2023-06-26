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

typedef std::map<std::string, std::list<std::string> > KeyValueMap;
typedef KeyValueMap::const_iterator MapIterator;
typedef std::list<std::string>::const_iterator ListIterator;

public:
    explicit Server(int port);
    Server();
    ~Server();
    void start();

private:
	
	int port;
    int serverSocket;
    std::string fileName;
    int 			createServerSocket();
    void 			bindServerSocket(int serverSocket, int port);
    int 			acceptClientConnection(int serverSocket);
    std::string 	extractBoundary(const std::string& data);
    bool 			saveFileContent(const std::string& data, const std::string& boundary);
    void 			sendResponse(int clientSocket, const std::string &response);
    void 			handleClientRequest(int clientSocket);
	void 			extractValues(const std::string &key, const KeyValueMap &keyValuePairs, std::string& value);
	void 			extractFilename(const std::string& key, const KeyValueMap &keyValuePairs);
	void 			printValueForKey(const std::string& key, const KeyValueMap &keyValuePairs);
	void 			parseRequest(const std::string& request, KeyValueMap &keyValuePairs);
	void			printMap(KeyValueMap &keyValuePairs);
	bool 			isASCII(const std::string& str);
	std::string		getRequestedFilename(const std::string &requestData);
	std::string		getDeletedFilename(const std::string &requestdata);
	std::string		loadStaticContent(const std::string &filename);
	std::string		loadStatic(void);
	void			handlePostRequest(int clientSocket, const std::string &requestData);
	void			handleGetRequest(int clientSocket, const std::string &requestData);
	void			handleDeleteRequest(int clientSocket, const std::string &requestData);

};

#endif

