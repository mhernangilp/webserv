#include "method.hpp"

static std::string getErrorPage(int error, const ServerConfig& serverConfig) {
    std::map<int, std::string>::const_iterator it = serverConfig.error_page.find(error);
    if (it != serverConfig.error_page.end()) {
        return serverConfig.root + it->second;
    } else {
        std::stringstream ss;
        ss << serverConfig.root << "error_pages/" << error << ".html";
        return ss.str();
    }
}

int try_delete(std::string url, int client_socket, Request request, const ServerConfig& serverConfig) {
    if (isDirectory(url)) {
        if (!checkdir(url, client_socket)) {
            sendHttpResponse(client_socket, "204 No Content", "text/html", "");
            request.setCode(204);
        } else {
            std::string body = getFileContent(getErrorPage(403, serverConfig));
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
        std::string body = getFileContent(getErrorPage(500, serverConfig));
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

    if (!serverConfig.isMethodAllowed(newUrl, 'D') || urlRecoil(request.getUrl())) {
        std::string notFoundPagePath = getErrorPage(405, serverConfig);
        std::string body = getFileContent(notFoundPagePath);
        if (body.empty())
            body = "<html><body><h1>405 Method Not Allowed</h1><p>DELETE is not allowed in this ubication.</p></body></html>";
        sendHttpResponse(client_socket, "405 Method Not Allowed", "text/html", body);
        request.setCode(405);
        close(client_socket);
        return (405);
    }

    newUrl = serverConfig.root + newUrl;

    if (access(newUrl.c_str(), F_OK) != -1) {
       int code = (try_delete(newUrl, client_socket, request, serverConfig));
       request.setCode(code);
    } else {
        std::string notFoundPagePath = getErrorPage(404, serverConfig);
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
