#include "server.hpp"
#include "server.cpp"

int main() {
    Server server(80);
    server.start();
    
    return 0;
}

