#include "method.hpp"

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

void sendHttpResponse(int client_socket, const std::string& statusCode, const std::string& contentType, const std::string& body) {
    std::string response = 
        "HTTP/1.1 " + statusCode + "\r\n"
        "Content-Type: " + contentType + "\r\n"
        "Content-Length: " + convertToString(body.size()) + "\r\n"
        "Connection: close\r\n"
        "\r\n" + body;
    
    send(client_socket, response.c_str(), response.size(), 0);
}

void try_delete(std::string url, int client_socket) {
    if (isDirectory(url)) {
        if (!checkdir(url, client_socket)) {
            sendHttpResponse(client_socket, "204 No Content", "text/html", "");
        } else {
            std::string body = getFileContent("docs/kebab_web/error_pages/403.html");
            if (body.empty()) {
                body = "<html><body><h1>403 Forbidden</h1><p>You don't have permission to access this resource.</p></body></html>";
            }
            sendHttpResponse(client_socket, "403 Forbidden", "text/html", body);
        }
    } else if (remove(url.c_str()) == 0) {
        sendHttpResponse(client_socket, "204 No Content", "text/html", "");
    } else {
        std::string body = getFileContent("docs/kebab_web/error_pages/500.html");
        if (body.empty()) {
            body = "<html><body><h1>500 Internal Server Error</h1><p>An error occurred while deleting the resource.</p></body></html>";
        }
        sendHttpResponse(client_socket, "500 Internal Server Error", "text/html", body);
    }
}



void deleteResponse(const std::string& url, int client_socket, const ServerConfig& serverConfig) {
    std::string newUrl;
    if (access(url.c_str(), F_OK) == -1)
        newUrl = urlDecode(url);

    if (!serverConfig.isDeleteAllowed(newUrl)) {
        std::string notFoundPagePath = "docs/kebab_web/error_pages/405.html";
        std::string body = getFileContent(notFoundPagePath);
        if (body.empty())
            body = "<html><body><h1>405 Method Not Allowed</h1><p>DELETE is not allowed in this ubication.</p></body></html>";
        sendHttpResponse(client_socket, "405 Method Not Allowed", "text/html", body);
        close(client_socket);
        return;
    }

    newUrl = serverConfig.root + newUrl;

    if (access(newUrl.c_str(), F_OK) != -1) {
       try_delete(newUrl, client_socket);
    } else {
        std::string notFoundPagePath = "docs/kebab_web/error_pages/404.html";
        std::string body = getFileContent(notFoundPagePath);

        if (body.empty()) {
            body = "<html><body><h1>404 Not Found</h1><p>The requested resource was not found.</p></body></html>";
        }
        sendHttpResponse(client_socket, "404 Not Found", "text/html", body);
    }
    close(client_socket);
}
