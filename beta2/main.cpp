#include "server.hpp"
#include "server.cpp"

int main() {
    Server server(8080);
    server.start();
    
    return 0;
}

