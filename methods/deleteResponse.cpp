#include "method.hpp"

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
