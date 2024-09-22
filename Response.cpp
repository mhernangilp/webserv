#include <fstream> 
#include <sstream>
#include <string> 
#include <iostream> 
#include <sys/socket.h>
#include <unistd.h>

std::string getContentType(const std::string& filePath) {
    if (filePath.find(".html") != std::string::npos) return "text/html";
    if (filePath.find(".css") != std::string::npos) return "text/css";
    if (filePath.find(".js") != std::string::npos) return "application/javascript";
    if (filePath.find(".png") != std::string::npos) return "image/png";
    if (filePath.find(".jpg") != std::string::npos || filePath.find(".jpeg") != std::string::npos) return "image/jpeg";
    if (filePath.find(".gif") != std::string::npos) return "image/gif";
    if (filePath.find(".svg") != std::string::npos) return "image/svg+xml";
    return "text/plain";  // Tipo de contenido por defecto
}

std::string getFileContent(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void handleGetRequest(const std::string& url, int client_socket) {
    std::string filePath;

    if (url == "/") {
        filePath = "docs/kebab_web/index.html";
    } else {
        filePath = "docs/kebab_web" + url;
    }

    // Leer el contenido del archivo solicitado
    std::string fileContent = getFileContent(filePath);

    if (fileContent.empty()) {
        // Si no se encuentra el archivo solicitado, cargar y devolver el archivo 404.html
        std::string notFoundPagePath = "docs/kebab_web/error_pages/404.html";
        std::string notFoundPageContent = getFileContent(notFoundPagePath);

        if (notFoundPageContent.empty()) {
            std::string notFoundResponse = 
                "HTTP/1.1 404 Not Found\r\n"
                "Content-Type: text/html\r\n"
                "Content-Length: 100\r\n"
                "\r\n"
                "<html><body><h1>404 Not Found</h1><p>The requested resource was not found.</p></body></html>";
            send(client_socket, notFoundResponse.c_str(), notFoundResponse.size(), 0);
        } else {
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
            "Content-Type: " + getContentType(filePath) + "\r\n"
            "Content-Length: " + std::to_string(fileContent.size()) + "\r\n"
            "\r\n" + fileContent;
        if (send(client_socket, httpResponse.c_str(), httpResponse.size(), 0) < 0) {
            std::cerr << "Error al enviar la respuesta: " << strerror(errno) << std::endl;
        }
    }
}
