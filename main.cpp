#include "server.cpp"

int main(void)
{
	int i;


	Server server;

	if(server.createSocket())
		return(1);
	server.configureSocket();
	if(server.linkSocket())
		return(1);
	if(server.listenSocket())
		return(1);
	server.createFileDescriptors();
	if(server.callSelect())
		return(1);
	server.run();
	return(0);
}


	
