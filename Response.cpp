#include <fstream> 
#include <sstream>
#include <string> 
#include <iostream> 
#include <sys/socket.h>
#include <unistd.h>

std::string getFileContent(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf(); // Leer el archivo completo en el buffer
    return buffer.str();    // Devolver el contenido del archivo como una cadena
}

void handleGetRequest(const std::string& url, int client_socket) {
    std::string filePath;

    if (url == "/") {
        filePath = "public/index.html";  // Página principal
    } else {
        filePath = "public" + url;  // Intentar cargar cualquier otro archivo dentro de la carpeta "public"
    }

    // Leer el contenido del archivo solicitado
    std::string fileContent = getFileContent(filePath);

    if (fileContent.empty()) {
        // Si no se encuentra el archivo solicitado, cargar y devolver el archivo 404.html
        std::string notFoundPagePath = "public/404.html";
        std::string notFoundPageContent = getFileContent(notFoundPagePath);

        if (notFoundPageContent.empty()) {
            // Si no se encuentra el archivo 404.html, devolver una respuesta genérica 404
            std::string notFoundResponse = 
                "HTTP/1.1 404 Not Found\r\n"
                "Content-Type: text/html\r\n"
                "Content-Length: 100\r\n"
                "\r\n"
                "<html><body><h1>404 Not Found</h1><p>The requested resource was not found.</p></body></html>";
            send(client_socket, notFoundResponse.c_str(), notFoundResponse.size(), 0);
        } else {
            // Si se encuentra 404.html, enviar su contenido como respuesta
            std::string notFoundResponse = 
                "HTTP/1.1 404 Not Found\r\n"
                "Content-Type: text/html\r\n"
                "Content-Length: " + std::to_string(notFoundPageContent.size()) + "\r\n"
                "\r\n" + notFoundPageContent;
            send(client_socket, notFoundResponse.c_str(), notFoundResponse.size(), 0);
        }
    } else {
        // Si se encuentra el archivo solicitado, devolver su contenido con un código 200 OK
        std::string httpResponse = 
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: " + std::to_string(fileContent.size()) + "\r\n"
            "\r\n" + fileContent;
        send(client_socket, httpResponse.c_str(), httpResponse.size(), 0);
    }
}
