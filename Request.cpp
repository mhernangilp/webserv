#include "Request.hpp"

Request::Request(){}

Request::Request(const std::string& raw_request) {
    parseRequest(raw_request);
}

void Request::parseRequest(const std::string& raw_request) {
    std::istringstream stream(raw_request);
    std::string line;

    // Leer la primera línea que contiene el método, la URL y la versión HTTP
    std::getline(stream, line);
    std::istringstream request_line(line);

    request_line >> method;
    request_line >> url;
    request_line >> http_version;

    // Parsear los headers
    while (std::getline(stream, line) && line != "\r") {
        size_t separator = line.find(": ");
        if (separator != std::string::npos) {
            std::string header_name = line.substr(0, separator);
            std::string header_value = line.substr(separator + 2);
            headers[header_name] = header_value;

            if (header_name == "Host") {
                host = header_value;
            }
        }
    }

    // Si hay un cuerpo en la solicitud (por ejemplo, en un POST), lo capturamos
    if (method == "POST" || method == "PUT") {
        std::getline(stream, line);
        body = line; // Capturar el cuerpo de la solicitud
    }
    //std::cout << "ESTO ES EL USEARR HEADER" << headers["User-Agent"] << std::endl;
}


std::string Request::getMethod() const { return method; }
std::string Request::getUrl() const { return url; }
std::string Request::getHttpVersion() const { return http_version; }
std::string Request::getHost() const { return host; }
std::map<std::string, std::string> Request::getHeaders() const { return headers; }
std::string Request::getBody() const { return body; }
