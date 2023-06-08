#include <iostream>
#include <string>

std::string obtenerContenidoDespuesDe(const std::string& cadena, const std::string& separador) {
    std::string::size_type pos = cadena.find(separador);
    std::string::size_type lastPos = std::string::npos;

    while (pos != std::string::npos) {
        lastPos = pos;
        pos = cadena.find(separador, pos + 1);
    }

    if (lastPos != std::string::npos) {
        return cadena.substr(lastPos + separador.length());
    }

    return "";
}

int main() {
    std::string cadena = "HOLAHOLA\r\n\r\nHOLAHOAHOLA\r\n\r\nADIOSADIOSADIOSADIOS";
    std::string separador = "\r\n\r\n";

    std::string contenidoDespues = obtenerContenidoDespuesDe(cadena, separador);

    if (!contenidoDespues.empty()) {
        std::cout << "Contenido después del último separador: " << contenidoDespues << std::endl;
    } else {
        std::cout << "No se encontró el separador en la cadena." << std::endl;
    }

    return 0;
}

