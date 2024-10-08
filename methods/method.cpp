#include "method.hpp"

void method(Request request, int socket, const ServerConfig& serverConfig){
	std::cout << "----------------------------------------------" << std::endl;
	std::cout << request.getMethod() << std::endl;
	std::cout << request.getUrl() << std::endl;
	std::cout << request.getHttpVersion() << std::endl;
	std::cout << request.getHost() << std::endl;
	std::cout << request.getHeaders()["User-Agent"] << std::endl;
	std::cout << request.getBody() << std::endl;
	std::cout << "----------------------------------------------" << std::endl;
						
	if (request.getMethod() == "GET")
		getResponse(request.getUrl(), socket, serverConfig);
	if (request.getMethod() == "DELETE")
		deleteResponse(request.getUrl(), socket, serverConfig);
	if (request.getMethod() == "POST")
		postResponse(request.getFileName(), request.getBody(), socket);
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