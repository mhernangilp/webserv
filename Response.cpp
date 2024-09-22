#include <fstream>  // Para manejar archivos
#include <sstream>  // Para construir la respuesta
#include <string>   // Para manejar cadenas
#include <iostream> // Para manejar la consola
#include <sys/socket.h>  // Para send y funciones relacionadas con sockets
#include <unistd.h>       // Para funciones POSIX como close()


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
        filePath = "public/index.html";
    } else {
        filePath = "public" + url;
    }

    std::string fileContent = getFileContent(filePath);

    if (fileContent.empty()) {
        std::string notFoundResponse = 
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: 100\r\n"
            "\r\n"
            "<html><body><h1>404 Not Found</h1><p>The requested resource was not found.</p></body></html>";
        send(client_socket, notFoundResponse.c_str(), notFoundResponse.size(), 0);
    } else {
        std::string httpResponse = 
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: " + std::to_string(fileContent.size()) + "\r\n"
            "\r\n" + fileContent;
        send(client_socket, httpResponse.c_str(), httpResponse.size(), 0);
    }
}
