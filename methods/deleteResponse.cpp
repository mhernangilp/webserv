#include "Request.hpp"
#include <dirent.h>
#include <sys/stat.h> // Para verificar si es un directorio
#include <unistd.h>   // Para usar remove

std::string FileContent(const std::string& filePath) {
    std::ifstream file(filePath.c_str(), std::ios::binary);
    if (!file.is_open()) {
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    return buffer.str();
}

std::string convertoString(int number) {
    std::stringstream ss;
    ss << number;
    return ss.str();
}

bool isDirectory(const std::string& path) {
    struct stat pathStat;
    stat(path.c_str(), &pathStat);
    return S_ISDIR(pathStat.st_mode);
}

int checkdir(const std::string& url, int client_socket) {
    DIR* dir = opendir(url.c_str());
    if (dir != NULL) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != NULL) {
            std::string fileName = entry->d_name;

            if (fileName != "." && fileName != "..") {
                std::string fullPath = url + "/" + fileName;

                if (isDirectory(fullPath)) {
                    checkdir(fullPath, client_socket);
                    if (rmdir(fullPath.c_str()) != 0) {
                        std::cerr << "Error eliminando el directorio: " << fullPath << std::endl;
                    }
                } else {
                    if (remove(fullPath.c_str()) != 0) {
                        std::cerr << "Error eliminando el archivo: " << fullPath << std::endl;
                    }
                }
            }
        }
        closedir(dir);
        if (rmdir(url.c_str()) != 0) {
            std::cerr << "Error eliminando el directorio: " << url << std::endl;
            return (1);
        }
    }
    return (0);
}

void try_delete(std::string resourcePath, int client_socket) {
    if (isDirectory(resourcePath)) {
        if (!checkdir(resourcePath, client_socket)){
            std::string response = "HTTP/1.1 204 No Content\r\n\r\n";
            send(client_socket, response.c_str(), response.size(), 0);
        }else {
            std::string notFoundPagePath = "docs/kebab_web/error_pages/403.html";
            std::string notFoundPageContent = FileContent(notFoundPagePath);
            if (notFoundPageContent.empty()) {
                std::string notFoundPageContent = "<html><body><h1>403 Forbidden</h1><p>You don't have permission to access this resource.</p></body></html>";
                std::string notFoundResponse = 
                     "HTTP/1.1 403 Forbidden\r\n"
                    "Content-Type: text/html\r\n"
                    "Content-Length: " + std::to_string(notFoundPageContent.size()) + "\r\n"
                    "Connection: close\r\n"
                    "\r\n" +
                    notFoundPageContent;
                send(client_socket, notFoundResponse.c_str(), notFoundResponse.size(), 0);
            } else {
                std::string notFoundResponse = 
                "HTTP/1.1 403 Forbidden\r\n"
                "Content-Type: text/html\r\n"
                "Content-Length: " + convertoString(notFoundPageContent.size() + 54) + "\r\n"
                "Connection: close\r\n"
                "\r\n" + notFoundPageContent;
                send(client_socket, notFoundResponse.c_str(), notFoundResponse.size(), 0);
            }
        } 
    }else if (remove(resourcePath.c_str()) == 0) {
        std::string response = "HTTP/1.1 204 No Content\r\n\r\n";
        send(client_socket, response.c_str(), response.size(), 0);
    } else {
        std::string notFoundPagePath = "docs/kebab_web/error_pages/500.html";
        std::string notFoundPageContent = FileContent(notFoundPagePath);
        if (notFoundPageContent.empty()) {
            std::string errorResponse = 
            "HTTP/1.1 500 Internal Server Error\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: 80\r\n"
            "Connection: close\r\n"
            "\r\n"
            "<html><body><h1>500 Internal Server Error</h1><p>An error occurred while deleting the resource.</p></body></html>";
            send(client_socket, errorResponse.c_str(), errorResponse.size(), 0);
        } else {
            std::string notFoundResponse = 
            "HTTP/1.1 500 Internal Server Error\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: " + convertoString(notFoundPageContent.size() + 54) + "\r\n"
            "Connection: close\r\n"
            "\r\n" + notFoundPageContent;
            send(client_socket, notFoundResponse.c_str(), notFoundResponse.size(), 0);
        }
    }
}

void deleteResponse(const std::string& url, int client_socket) {
    std::string resourcePath;

    if (url.substr(0, 11) == "/up_to_you/") {
        resourcePath = "docs/kebab_web" + url;
    } else {
        resourcePath = "docs/kebab_web/up_to_you" + url;
    }
    if (access(resourcePath.c_str(), F_OK) != -1) {
       try_delete(resourcePath, client_socket);
    } else {
        std::string notFoundPagePath = "docs/kebab_web/error_pages/404.html";
        std::string notFoundPageContent = FileContent(notFoundPagePath);

        if (notFoundPageContent.empty()) {
            std::string notFoundPageContent = "<html><body><h1>404 Not Found</h1><p>The requested resource was not found.</p></body></html>";
            std::string notFoundResponse = 
                "HTTP/1.1 404 Not Found\r\n"
                "Content-Type: text/html\r\n"
                "Content-Length: " + std::to_string(notFoundPageContent.size()) + "\r\n"
                "Connection: close\r\n"
                "\r\n" +
                notFoundPageContent;
            send(client_socket, notFoundResponse.c_str(), notFoundResponse.size(), 0);
        } else {
            std::string notFoundResponse = 
                "HTTP/1.1 404 Not Found\r\n"
                "Content-Type: text/html\r\n"
                "Content-Length: " + convertoString(notFoundPageContent.size() + 54) + "\r\n"
                "Connection: close\r\n"
                "\r\n" + notFoundPageContent;
            send(client_socket, notFoundResponse.c_str(), notFoundResponse.size(), 0);
        }
    }
}
