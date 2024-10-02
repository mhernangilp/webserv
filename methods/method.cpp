#include "Method.hpp"


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