#include "method.hpp"
#include "Request.hpp"
#include "../Server.hpp"
#include <sys/stat.h>
#include <iostream>
#include <dirent.h>
#include <string>
#include <vector>
#include <sstream>
#include "../Utils.hpp"

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

std::string generateAutoIndexPage(const std::string& dirPath, const std::string& urlPath) {
    DIR *dir;
    struct dirent *ent;
    std::ostringstream html;

    // Generar el encabezado de la página
    html << "<html><head><title>Index of " << urlPath << "</title></head><body>";
    html << "<h1>Index of " << urlPath << "</h1>";
    html << "<hr><pre>";

    if ((dir = opendir(dirPath.c_str())) != NULL) {
        // Añadir el enlace para subir al directorio anterior
        html << "<a href=\"../\">../</a>\n";

        // Iterar sobre los archivos y directorios en el directorio actual
        while ((ent = readdir(dir)) != NULL) {
            std::string name(ent->d_name);
            
            if (name != "." && name != "..") {
                // Si es un directorio, añadimos '/' al final del nombre
                if (ent->d_type == DT_DIR) {
                    name += "/";
                }

                // Generamos la línea para cada archivo o directorio
                html << "<a href=\"" << urlPath + "/" +name << "\">" << name << "</a>\n";
            }
        }
        closedir(dir);
    } else {
        html << "Directory not found\n";
    }

    html << "</pre><hr></body></html>";
    return html.str();
}

bool isAutoindexDefined(const LocationConfig& loc) {
    return loc.autoindex == "on" || loc.autoindex == "off";
}

std::string getRelativePath(const std::string& path, const std::string& root) {
    if (path.find(root) == 0) {
        return path.substr(root.length() - 1);
    }
    return path;
}

bool autoindex_allowed(std::string path, const ServerConfig& serverConfig) {
    std::string relativePath = getRelativePath(path, serverConfig.root);

    while (relativePath != "/") {
        std::map<std::string, LocationConfig>::const_iterator it = serverConfig.locations.find(relativePath);
        if (it != serverConfig.locations.end()) {
            if (isAutoindexDefined(it->second)) {
                return it->second.autoindex == "on";
            }
        }

        size_t lastSlash = relativePath.find_last_of('/');
        if (lastSlash != std::string::npos && lastSlash != 0) {
            relativePath = relativePath.substr(0, lastSlash);
        } else {
            break;
        }
    }

    std::map<std::string, LocationConfig>::const_iterator itRoot = serverConfig.locations.find("/");
    if (itRoot != serverConfig.locations.end()) {
        if (isAutoindexDefined(itRoot->second)) {
            return itRoot->second.autoindex == "on";
        }
    }

    return false;
}

int exc_script(Request request, const ServerConfig& serverConfig, int client_socket, std::string name, std::string exists){
    std::string script_path = serverConfig.root + "/cgi-bin/checker.php";
    std::string response_body;

    std::string command = "php " + script_path + " " + name + " " + exists;

    pid_t pid = fork();
    
    if (pid < 0) {
        std::cerr << "Fork failed!" << std::endl;
        std::string response_body = getFileContent(getErrorPage(500, serverConfig));
        if (response_body.empty())
            std::string response_body = "<html><body><h1>500 Internal Server Error</h1><p>Fork failed.</p></body></html>";
        sendHttpResponse(client_socket, "500 Internal Server Error", "text/html", response_body);
        request.setCode(500);
        return 500;
    }

    if (pid == 0) {
        FILE* pipe = popen(command.c_str(), "r");
        
        if (pipe) {
            char buffer[128];
            char *fg = fgets(buffer, sizeof(buffer), pipe);
            
            while (fg != NULL) {
                response_body += buffer;
                fg = fgets(buffer, sizeof(buffer), pipe);
            }

            int status = pclose(pipe);
            if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
                std::string response_body = getFileContent(getErrorPage(500, serverConfig));
                if (response_body.empty())
                    std::string response_body = "<html><body><h1>500 Internal Server Error</h1><p>Could not open file for writing.</p></body></html>";
                sendHttpResponse(client_socket, "500 Internal Server Error", "text/html", response_body);
                request.setCode(500);
                exit(1);
                return (500);
            }
            sendHttpResponse(client_socket, "200 OK", "text/html", response_body);
            exit(0);
        } else {
            std::string response_body = getFileContent(getErrorPage(500, serverConfig));
            if (response_body.empty())
                std::string response_body = "<html><body><h1>500 Internal Server Error</h1><p>Could not open pipe to execute script.</p></body></html>";
            sendHttpResponse(client_socket, "500 Internal Server Error", "text/html", response_body);
            exit(1);
        }
    } else {
        int status;
        time_t start_time = time(NULL);
        while (true) {
            int result = waitpid(pid, &status, WNOHANG);
            if (result == pid) {
                break;
            }

            if (time(NULL) - start_time > 5) {
                kill(pid, SIGKILL);
                std::string response_body = getFileContent(getErrorPage(500, serverConfig));
                if (response_body.empty())
                    std::string response_body = "<html><body><h1>500 Internal Server Error</h1><p>Script execution timed out.</p></body></html>";
                sendHttpResponse(client_socket, "500 Internal Server Error", "text/html", response_body);
                request.setCode(500);
                return 500;
            }
            usleep(100000);
        }
        return request.getCode();
    }
}

int cgi_char(const std::string& str, char ch) {
    for (int i = str.size() - 1; i >= 0; --i) {
        if (str[i] == ch) {
            return i;
        }
    }
    return -1;
}

int check_reps(std::string string){
    int ump = 0;
    int sign = 0;

    if (string.size() < 7 || string[0] == '=')
        return (-1);
    for (int i = 0; string[i]; i++){
        if (string[i] == '=')
            sign++;
        if (string[i] == '&'){
            ump++;
            if (string[i + 1] == '=')
                return -1;
        }
    }
    if (ump != 1 || sign != 2)
        return -1;
    return 0;
}

// http://localhost:8002/cgi-bin/checker.php?filename=CV.pdf&exists=NO
int cgi(Request request, const ServerConfig& serverConfig, int client_socket){
    const std::string url = serverConfig.root + request.getUrl();
    std::string new_url;

    size_t pos = url.find('?');
    if (pos != std::string::npos){
        new_url = url.substr(0, pos);
            if (fileExists(new_url)){
                int i = cgi_char(new_url.c_str(), '/');
                std::string file = new_url.substr(i, new_url.size());
                if (file == "/checker.php"){
                    std::string variables = url.substr(pos + 1, url.size() - pos - 1);

                    if (!check_reps(variables)){
                        std::string v1 = variables.substr(variables.find('=') + 1, variables.find('&') - variables.find('=') - 1);
                        std::string v2 = variables.substr(cgi_char(variables.c_str(), '=') + 1, variables.size());
                        if (v1.size() > 0 && v2.size() > 0){
                            request.setCode(exc_script(request, serverConfig, client_socket, v1, v2));
                            return (request.getCode());
                        }
                    }
                    
                    std::string response_body = getFileContent(getErrorPage(400, serverConfig));
                    if (response_body.empty())
                        std::string response_body = "<html><body><h1>400 Bad Request</h1><p>No filename specified.</p></body></html>";
                    sendHttpResponse(client_socket, "400 Bad Request", "text/html", response_body);
                    request.setCode(400);
                    return (400);
                }
            }
    }
    return -1;
}

int getResponse(Request request, int client_socket, const ServerConfig& serverConfig) {
    std::string filePath;
    struct stat fileStat;
    std::string url = request.getUrl();
    std::string newUrl = url;

    // Eliminar barras adicionales al final de la URL
    while (url.size() > 1 && url[url.size() - 1] == '/') {
        url.erase(url.size() - 1);
    }

    // Si la URL inicial era diferente de la URL limpiada, hacer una redirección
    if (url != request.getUrl()) {
        std::string redirectResponse =
            "HTTP/1.1 301 Moved Permanently\r\n"
            "Location: " + url + "\r\n"
            "Content-Length: 0\r\n"
            "Connection: close\r\n"
            "\r\n";
        send(client_socket, redirectResponse.c_str(), redirectResponse.size(), 0);
        request.setCode(301);
        close(client_socket);
        return 301;
    }
    
    if (access(url.c_str(), F_OK) == -1)
        newUrl = urlDecode(url);
    if (!serverConfig.isMethodAllowed(newUrl, 'G') || urlRecoil(request.getUrl())) {
        std::string response_body = getFileContent(getErrorPage(405, serverConfig));
        if (response_body.empty())
            response_body = "<html><body><h1>405 Method Not Allowed</h1><p>POST is not allowed in this ubication.</p></body></html>";
        sendHttpResponse(client_socket, "405 Method Not Allowed", "text/html", response_body);
        request.setCode(405);
        close(client_socket);
        return (405);
    }

    newUrl = normalizeUrl(newUrl);

    // Return to specific path if present in config file
    for (std::map<std::string, LocationConfig>::const_iterator it = serverConfig.locations.begin(); it != serverConfig.locations.end(); ++it) {
        if (it->first == newUrl) {
            if (it->second.return_path != "") {
                // Enviar una respuesta de redirección
                std::string redirectResponse =
                    "HTTP/1.1 302 Found\r\n"
                    "Location: " + it->second.return_path + "\r\n"
                    "Content-Length: 0\r\n"
                    "Connection: close\r\n"
                    "\r\n";
                send(client_socket, redirectResponse.c_str(), redirectResponse.size(), 0);
                request.setCode(302);
                close(client_socket);
                return request.getCode();
            }
        }
    }

    // Replace root of location
    for (std::map<std::string, LocationConfig>::const_iterator it = serverConfig.locations.begin(); it != serverConfig.locations.end(); ++it) {
        if (!it->second.root.empty()) {
            std::string old_url = newUrl;
            if (old_url.find(it->first) == 0) {
                old_url.replace(0, it->first.length(), it->second.root);
                newUrl = old_url;
            }
        }
    }

    if (newUrl == "/") {
        filePath = serverConfig.root + "index.html";
    } else {
        filePath = serverConfig.root + newUrl;
    }

    filePath = removeDuplicateSlashes(filePath);

    // Verificar si la ruta es un directorio
    if (stat(filePath.c_str(), &fileStat) == 0 && S_ISDIR(fileStat.st_mode)) {
        // Si es un directorio, verificar si tiene un index.html
        std::string indexPath = filePath + "/index.html";
        
        // Check for custom index
        std::string remainingPath = filePath.substr(serverConfig.root.length() - 1);
        for (std::map<std::string, LocationConfig>::const_iterator it = serverConfig.locations.begin(); it != serverConfig.locations.end(); ++it) {
            if (!it->second.index.empty() && (it->first == remainingPath || (!it->second.root.empty() && it->second.root == remainingPath))) {
                indexPath = filePath + "/" +it->second.index;
            }
        }
        
        std::string indexContent = getFileContent(indexPath);

        if (indexContent.empty() && !autoindex_allowed(filePath, serverConfig)) {
            // Retornar un error 403 Forbidden
            std::string forbiddenPagePath =  getErrorPage(403, serverConfig);
            std::string forbiddenPageContent = getFileContent(forbiddenPagePath);

            if (forbiddenPageContent.empty()) {
                // Si no hay una página personalizada para el error 403, usar un mensaje predeterminado
                forbiddenPageContent = "<html><body><h1>403 Forbidden</h1><p>Access to this directory is forbidden.</p></body></html>";
            }

            std::string forbiddenResponse =
                "HTTP/1.1 403 Forbidden\r\n"
                "Content-Type: text/html\r\n"
                "Content-Length: " + convertToString(forbiddenPageContent.size()) + "\r\n"
                "Connection: close\r\n"
                "\r\n" + forbiddenPageContent;
            send(client_socket, forbiddenResponse.c_str(), forbiddenResponse.size(), 0);
            request.setCode(403);
        } else if (indexContent.empty() && autoindex_allowed(filePath, serverConfig)) {
            // Si no hay un archivo index.html pero el autoindex está habilitado, generar el autoindex
            std::string autoIndexContent = generateAutoIndexPage(filePath, url);
            std::string autoIndexResponse =
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/html\r\n"
                "Content-Length: " + convertToString(autoIndexContent.size()) + "\r\n"
                "Connection: close\r\n"
                "\r\n" + autoIndexContent;
            send(client_socket, autoIndexResponse.c_str(), autoIndexResponse.size(), 0);
            request.setCode(200);
        } else {
            // Si existe el index.html, devolverlo
            std::string httpResponse =
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: " + getContentType(indexPath) + "\r\n"
                "Content-Length: " + convertToString(indexContent.size()) + "\r\n"
                "Connection: close\r\n"
                "\r\n" + indexContent;
            send(client_socket, httpResponse.c_str(), httpResponse.size(), 0);
            request.setCode(200);
        }
    } else {
        // Si no es un directorio, intentar devolver el archivo solicitado
        std::string fileContent = getFileContent(filePath);
        
        if (fileContent.empty()) {
            int script = cgi(request, serverConfig, client_socket);
            if (script != -1){
                close (client_socket);
                return (script);
            }
            if (fileExists(filePath)){
                std::string contentType = getContentType(filePath);
                sendHttpResponse(client_socket, "200 OK", contentType, "");
                close (client_socket);
                request.setCode(200);
                return (request.getCode());
            }

            std::string notFoundPagePath = getErrorPage(404, serverConfig);
            std::string notFoundPageContent = getFileContent(notFoundPagePath);

            if (notFoundPageContent.empty()) {
                std::string notFoundPageContent = "<html><body><h1>404 Not Found</h1><p>The requested resource was not found.</p></body></html>";
                std::string notFoundResponse = 
                    "HTTP/1.1 404 Not Found\r\n"
                    "Content-Type: text/html\r\n"
                    "Content-Length: " + convertToString(notFoundPageContent.size()) + "\r\n"
                    "Connection: close\r\n"
                    "\r\n" +
                    notFoundPageContent;
                send(client_socket, notFoundResponse.c_str(), notFoundResponse.size(), 0);
                request.setCode(404);
            } else {
                std::string notFoundResponse = 
                    "HTTP/1.1 404 Not Found\r\n"
                    "Content-Type: text/html\r\n"
                    "Content-Length: " + convertToString(notFoundPageContent.size()) + "\r\n"
                    "Connection: close\r\n"
                    "\r\n" + notFoundPageContent;
                send(client_socket, notFoundResponse.c_str(), notFoundResponse.size(), 0);
                request.setCode(404);
            }
        } else {
            // Si se encuentra el archivo solicitado, devolver su contenido con un código 200 OK
            std::string httpResponse =
            "HTTP/1.1 200 OK\r\n"
                "Content-Type: " + getContentType(filePath) + "\r\n"
                "Content-Length: " + convertToString(fileContent.size()) + "\r\n"
                "Connection: close\r\n"
                "\r\n" + fileContent;
            send(client_socket, httpResponse.c_str(), httpResponse.size(), 0);
            request.setCode(200);
        }
    }
    close (client_socket);
    return (request.getCode());
}
