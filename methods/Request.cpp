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

    code = 0;
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

    if (method == "POST") {
        std::size_t pos = raw_request.find("\r\n\r\n");

        if (pos != std::string::npos) {
            body = raw_request.substr(pos + 4);

            // Buscar el boundary
            std::string boundaryMarker = "------WebKitFormBoundary"; // Cambia esto si tu boundary tiene un nombre diferente
            std::size_t boundaryPos = body.find(boundaryMarker);

            if (boundaryPos != std::string::npos) {

                std::size_t nextBoundaryPos = body.find(boundaryMarker, boundaryPos + boundaryMarker.length());
                std::size_t filePartEnd = (nextBoundaryPos != std::string::npos) ? nextBoundaryPos : body.length();

                // Extraer la parte del archivo, que está después de la cabecera
                std::size_t filePartStart = boundaryPos + boundaryMarker.length(); // Moverse justo después del boundary
                std::size_t headerEnd = body.find("\r\n\r\n", filePartStart); // Encontrar el final de la cabecera

                if (headerEnd != std::string::npos && filePartEnd > headerEnd) {
                    std::string filePart = body.substr(headerEnd + 4, filePartEnd - (headerEnd + 4));
                    std::string header = body.substr(boundaryPos, headerEnd - boundaryPos);
                    body = filePart;
                    std::size_t filenamePos = header.find("filename=\"");
                    if (filenamePos != std::string::npos) {
                        filenamePos += 10; // Moverse después de "filename=\""
                        std::size_t endQuotePos = header.find("\"", filenamePos);
                        if (endQuotePos != std::string::npos) {
                            this->file_name = header.substr(filenamePos, endQuotePos - filenamePos);
                        } else {
                            std::cerr << "End quote for filename not found!" << std::endl;
                            code = 400;
                        }
                    } else {
                        std::cerr << "Filename not found!" << std::endl;
                        code = 400;
                    }
                } else {
                    std::cerr << "Header end not found or invalid range!" << std::endl;
                    code = 401;
                }
            } else {
                std::cerr << "Boundary not found in the body!" << std::endl;
                code = 415;
            }
        } else {
            std::cerr << "No headers found!" << std::endl;
            code = 401;
        }
    }
}


std::string Request::getMethod() const { return method; }
std::string Request::getUrl() const { return url; }
std::string Request::getHttpVersion() const { return http_version; }
std::string Request::getHost() const { return host; }
std::map<std::string, std::string> Request::getHeaders() const { return headers; }
std::string Request::getBody() const { return body; }
std::string Request::getFileName() const { return file_name; }
int Request::getCode() { return code; }
void Request::setCode(int code) { this->code = code; }
void Request::setUrl(std::string url) { this->url = url; }
