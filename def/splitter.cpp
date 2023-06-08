#include <iostream>
#include <string>

std::string obtenerAntesDelSeparador(const std::string& cadena, const std::string& separador) {
    std::size_t pos = cadena.find(separador);
    if (pos != std::string::npos) {
        return cadena.substr(0, pos);
    }
    return cadena;
}

int main() {
    std::string cadenaCompleta = "Parte antes del separador\r\n\r\nParte después del separador";
    std::string separador = "\r\n\r\n";
    std::string parteAnterior = obtenerAntesDelSeparador(cadenaCompleta, separador);
    std::cout << "Parte anterior al separador:\n" << parteAnterior << std::endl;

    return 0;
}

