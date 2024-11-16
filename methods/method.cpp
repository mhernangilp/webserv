#include "method.hpp"
#include "../Server.hpp"

void method(Request request, int socket, const ServerConfig& serverConfig) {
    if (request.getMethod() == "GET" || request.getMethod() == "DELETE" || request.getMethod() == "POST"){
        int code = 0;			
        if (request.getMethod() == "GET")
            code = getResponse(request, socket, serverConfig);
        else if (request.getMethod() == "DELETE")
            code = deleteResponse(request, socket, serverConfig);
        else if (request.getMethod() == "POST")
            code = postResponse(request, socket, serverConfig);

        std::cout << BLUE << "[INFO] Response sent to client " << socket - 3 << ", Stats " << code << RESET << std::endl;
    }
}

void body_limit(int client_socket, const ServerConfig& serverConfig){
    std::string response_body = getFileContent(getErrorPage(413, serverConfig));
        if (response_body.empty())
            std::string response_body = "<html><body><h1>413 Payload Too Large</h1><p>Body size limit.</p></body></html>";
        sendHttpResponse(client_socket, "413 Payload Too Large", "text/html", response_body);
        std::cout << BLUE << "[INFO] Response sent to client " << client_socket << ", Stats " << 413 << RESET << std::endl;
        return ;
}

std::string getFileContent(const std::string& filePath) {
    std::ifstream file(filePath.c_str());
    if (!file.is_open()) {
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    return buffer.str();
}

std::string convertToString(int number) {
    std::stringstream ss;
    ss << number;
    return ss.str();
}

std::string urlDecode(const std::string& url) {
    std::ostringstream decoded;
    for (size_t i = 0; i < url.size(); ++i) {
        if (url[i] == '%' && i + 2 < url.size()) {
            std::istringstream hex(url.substr(i + 1, 2));
            int value;
            if (hex >> std::hex >> value) {
                decoded << static_cast<char>(value);
                i += 2;
            }
        } else if (url[i] == '+') {
            decoded << ' ';
        } else {
            decoded << url[i];
        }
    }
    return decoded.str();
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

void sendHttpResponse(int client_socket, const std::string& statusCode, const std::string& contentType, const std::string& body) {
    std::string response = 
        "HTTP/1.1 " + statusCode + "\r\n"
        "Content-Type: " + contentType + "\r\n"
        "Content-Length: " + convertToString(body.size()) + "\r\n"
        "Connection: close\r\n"
        "\r\n" + body;
    
    send(client_socket, response.c_str(), response.size(), 0);
}

bool urlRecoil(const std::string &url){
    for (int i = 0; url[i]; i++){
        if (url[i] == '/' && url[i + 1] && url[i + 1] == '.'){
            if (url[i + 2] == '\0' || url[i + 2] == '/')
                return true;
            else if (url[i + 2] && url[i + 2] == '.' && (url[i + 3] == '\0' || url[i + 3] == '/'))
                return true;
        }
    }
    return false;
}

std::string getErrorPage(int error, const ServerConfig& serverConfig) {
    std::map<int, std::string>::const_iterator it = serverConfig.error_page.find(error);
    if (it != serverConfig.error_page.end()) {
        return serverConfig.root + it->second;
    } else {
        std::stringstream ss;
        ss << serverConfig.root << "error_pages/" << error << ".html";
        return ss.str();
    }
}

bool fileExists(const std::string& filename) {
    struct stat buffer;
    return (stat(filename.c_str(), &buffer) == 0);
}