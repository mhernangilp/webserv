#include "Request.hpp"
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <cstring>

Request::Request() {}

Request::Request(const std::string& raw_request) {
    parseRequest(raw_request);
}

void Request::parseRequest(const std::string& raw_request) {
    std::istringstream stream(raw_request);
    std::string line;

    // Leer la línea de la solicitud
    std::getline(stream, line);
    std::istringstream request_line(line);
    request_line >> method;
    request_line >> url;
    request_line >> http_version;

    // Leer los headers
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

    // Si el método es POST, maneja el cuerpo
    if (method == "POST") {
        if (headers.find("Content-Length") != headers.end()) {
            size_t content_length = std::stoi(headers["Content-Length"]);
            body.resize(content_length); // Preasigna el tamaño del cuerpo
            stream.read(&body[0], content_length); // Lee el cuerpo en el string
            // Aquí puedes decidir si quieres hacer algo con el cuerpo, como almacenarlo en un archivo
        } else if (headers.find("Transfer-Encoding") != headers.end() && headers["Transfer-Encoding"] == "chunked") {
            parseChunkedBody(stream);
        }
        std::cout << body << std::endl;
    }
}

void Request::parseChunkedBody(std::istringstream& stream) {
    std::string chunk_size_str;
    while (std::getline(stream, chunk_size_str)) {
        size_t chunk_size = std::stoul(chunk_size_str, nullptr, 16); // Convertir el tamaño del chunk de hexadecimal a decimal
        if (chunk_size == 0) break; // Si el tamaño es 0, hemos terminado

        std::string chunk_data(chunk_size, '\0'); // Crear un buffer para el chunk
        stream.read(&chunk_data[0], chunk_size); // Leer el chunk
        body.append(chunk_data); // Agregarlo al cuerpo

        // Leer la línea final del chunk (normalmente es CRLF)
        std::getline(stream, chunk_size_str); 
    }
}

std::string Request::getMethod() const { return method; }
std::string Request::getUrl() const { return url; }
std::string Request::getHttpVersion() const { return http_version; }
std::string Request::getHost() const { return host; }
std::map<std::string, std::string> Request::getHeaders() const { return headers; }
std::string Request::getBody() const { return body; }
