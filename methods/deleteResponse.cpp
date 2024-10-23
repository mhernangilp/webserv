#include "method.hpp"

int try_delete(std::string url, int client_socket, Request request) {
    if (isDirectory(url)) {
        if (!checkdir(url, client_socket)) {
            sendHttpResponse(client_socket, "204 No Content", "text/html", "");
            request.setCode(204);
        } else {
            std::string body = getFileContent("docs/kebab_web/error_pages/403.html");
            if (body.empty()) {
                body = "<html><body><h1>403 Forbidden</h1><p>You don't have permission to access this resource.</p></body></html>";
            }
            sendHttpResponse(client_socket, "403 Forbidden", "text/html", body);
            request.setCode(403);
        }
    } else if (remove(url.c_str()) == 0) {
        sendHttpResponse(client_socket, "204 No Content", "text/html", "");
        request.setCode(204);
    } else {
        std::string body = getFileContent("docs/kebab_web/error_pages/500.html");
        if (body.empty()) {
            body = "<html><body><h1>500 Internal Server Error</h1><p>An error occurred while deleting the resource.</p></body></html>";
        }
        sendHttpResponse(client_socket, "500 Internal Server Error", "text/html", body);
        request.setCode(500);
    }
    return (request.getCode());
}



int deleteResponse(Request request, int client_socket, const ServerConfig& serverConfig) {
    const std::string& url = request.getUrl();
    std::string newUrl = url;
    if (access(url.c_str(), F_OK) == -1)
        newUrl = urlDecode(url);

   /* if (!serverConfig.isDeleteAllowed(newUrl)) {
        std::string notFoundPagePath = "docs/kebab_web/error_pages/405.html";
        std::string body = getFileContent(notFoundPagePath);
        if (body.empty())
            body = "<html><body><h1>405 Method Not Allowed</h1><p>DELETE is not allowed in this ubication.</p></body></html>";
        sendHttpResponse(client_socket, "405 Method Not Allowed", "text/html", body);
        request.setCode(405);
        close(client_socket);
        return (405);
    }
*/
    newUrl = serverConfig.root + newUrl;

    if (access(newUrl.c_str(), F_OK) != -1) {
       int code = (try_delete(newUrl, client_socket, request));
       request.setCode(code);
    } else {
        std::string notFoundPagePath = "docs/kebab_web/error_pages/404.html";
        std::string body = getFileContent(notFoundPagePath);

        if (body.empty()) {
            body = "<html><body><h1>404 Not Found</h1><p>The requested resource was not found.</p></body></html>";
        }
        sendHttpResponse(client_socket, "404 Not Found", "text/html", body);
        request.setCode(404);
    }
    close(client_socket);
    return (request.getCode());
}
