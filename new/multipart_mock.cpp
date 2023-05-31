#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

void SaveFileContent(const std::string& filename, const std::string& content) {
    std::ofstream file(filename.c_str(), std::ios::out | std::ios::binary);
    if (file) {
        file.write(content.c_str(), content.length());
        file.close();
        std::cout << "Archivo guardado: " << filename << std::endl;
    } else {
        std::cerr << "No se pudo guardar el archivo: " << filename << std::endl;
    }
}

void ParseMultipartFormData(const std::string& requestData) {
    // Encontrar el separador de contenido
    std::string boundary;
    std::size_t pos = requestData.find("boundary=");
    if (pos != std::string::npos) {
        boundary = requestData.substr(pos + 9);
    } else {
        std::cerr << "Error: No se encontró el separador de contenido" << std::endl;
        return;
    }

    // Separar las partes del cuerpo de la solicitud
    std::size_t start = requestData.find("\r\n\r\n");
    std::size_t end = requestData.rfind("--" + boundary + "--");
    if (start != std::string::npos && end != std::string::npos) {
        std::string body = requestData.substr(start + 4, end - start - 4);

        // Analizar cada parte del cuerpo
        std::size_t partStart = 0;
        while ((partStart = body.find("\r\n\r\n", partStart)) != std::string::npos) {
            partStart += 4;
            std::size_t partEnd = body.find("\r\n--" + boundary, partStart);
            if (partEnd != std::string::npos) {
                std::string part = body.substr(partStart, partEnd - partStart);

                // Buscar el nombre de archivo y el contenido del archivo
                std::size_t filenameStart = part.find("filename=\"");
                if (filenameStart != std::string::npos) {
                    std::size_t filenameEnd = part.find("\"", filenameStart + 10);
                    if (filenameEnd != std::string::npos) {
                        std::string filename = part.substr(filenameStart + 10, filenameEnd - filenameStart - 10);

                        // Buscar el contenido del archivo
                        std::size_t contentStart = part.find("\r\n\r\n");
                        if (contentStart != std::string::npos) {
                            std::string content = part.substr(contentStart + 4);

                            // Guardar el contenido en un archivo
                            SaveFileContent(filename, content);
                        }
                    }
                }
            }
        }
    } else {
        std::cerr << "Error: Formato de solicitud no válido" << std::endl;
    }
}

int main() {
    std::string requestData = "POST / HTTP/1.1\r\n"
                              "Host: localhost:8080\r\n"
                              "User-Agent: curl/7.64.1\r\n"
                              "Accept: */*\r\n"
                              "Content-Length: 193\r\n"
                              "Content-Type: multipart/form-data; boundary=------------------------6f1f8de5f025bbc3\r\n"
                              "\r\n"
                              "--------------------------6f1f8de5f025bbc3\r\n"
                              "Content-Disposition: form-data; name=\"file\"; filename=\"prueba.txt\"\r\n"
                              "Content-Type: text/plain\r\n"
                              "\r\n"
                              "Test\r\n"
                              "\r\n"
                              "--------------------------6f1f8de5f025bbc3--\r\n";

    ParseMultipartFormData(requestData);

    return 0;
}

