#include "server.hpp"
#include "server.cpp"

int main() {
    Server server(443);
    server.start();
    
    return 0;
}

