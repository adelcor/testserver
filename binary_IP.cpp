#include <iostream>
#include <arpa/inet.h>

int main() {
    const char* ip_address = "127.0.0.1";
    uint32_t binary_address = inet_addr(ip_address);
    if (binary_address == INADDR_NONE) {
        std::cout << "Error al convertir dirección IP" << std::endl;
        return 1;
    }
    std::cout << "Dirección IP en binario: " << &binary_address << std::endl;
    return 0;
}

