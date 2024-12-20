#include "Request.hpp"
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <cstring>

Request::Request() : method(""), url(""), http_version(""), host(""), headers(), body(""), file_name(""), file_route(""), code(0), server(0), valid(true) {}

Request::Request(const std::string raw_request) : valid(true) {
    parseRequest(raw_request);
}

Request::Request(const Request& other)
    : method(other.method),
      url(other.url),
      http_version(other.http_version),
      host(other.host),
      headers(other.headers),
      body(other.body),
      file_name(other.file_name),
      file_route(other.file_route),
      code(0),
      server(0),
      valid(true)
{}


Request& Request::operator=(const Request& other) {
    if (this != &other) {
        this->method = other.method;
        this->url = other.url;
        this->http_version = other.http_version;
        this->host = other.host;
        this->headers = other.headers;
        this->body = other.body;
        this->file_name = other.file_name;
        this->file_route = other.file_route;
        this->code = 0;
        this->server = 0;
        this->valid = true;
    }
    return *this;
}


void Request::parseRequest(const std::string raw_request) {
    std::istringstream stream(raw_request);
    std::string line;

    code = 0;

    std::getline(stream, line);
    std::istringstream request_line(line);
    request_line >> method >> url >> http_version;

    // Leer headers
    while (std::getline(stream, line) && line != "\r") {
        size_t separator = line.find(": ");
        if (separator != std::string::npos) {
            std::string header_name = line.substr(0, separator);
            std::string header_value = line.substr(separator + 2);
            headers[header_name] = header_value;
        }
    }

    if (method == "POST") {
        // Buscar el inicio del cuerpo
        std::size_t pos = raw_request.find("\r\n\r\n");
        if (pos == std::string::npos) {
            std::cerr << "No headers found!" << std::endl;
            code = 401;
            return;
        }
        body = raw_request.substr(pos + 4);

        // Procesar multipart/form-data
        std::string boundaryMarker = "--" + headers["Content-Type"].substr(headers["Content-Type"].find("boundary=") + 9);
        std::size_t boundaryPos = body.find(boundaryMarker);

        if (boundaryPos == std::string::npos) {
            std::cerr << "Boundary not found in body!" << std::endl;
            code = 415;
            return;
        }

        // Extraer el archivo
        std::size_t fileHeaderEnd = body.find("\r\n\r\n", boundaryPos);
        if (fileHeaderEnd != std::string::npos) {
            std::string fileHeader = body.substr(boundaryPos, fileHeaderEnd - boundaryPos);
            std::size_t fileStart = fileHeaderEnd + 4;
            std::size_t nextBoundary = body.find(boundaryMarker, fileStart);
            std::string fileContent = body.substr(fileStart, nextBoundary - fileStart);
            boundaryPos = nextBoundary;
        }

        // Extraer el campo `route`
        std::size_t routeHeaderStart = body.find(boundaryMarker, boundaryPos);
        std::string aux_body = raw_request;
        if (routeHeaderStart != std::string::npos) {
            std::size_t routeHeaderEnd = aux_body.find("\r\n\r\n", routeHeaderStart);
            std::size_t routeStart = routeHeaderEnd + 4;
            std::size_t nextBoundary = aux_body.find(boundaryMarker, routeStart);
            std::string routeContent = aux_body.substr(routeStart, nextBoundary - routeStart);
            routeContent.erase(routeContent.find_last_not_of("\r\n") + 1);
            file_route = routeContent;
            file_route = file_route.substr(0, file_route.find("\r\n"));
            if (file_route[0] == '/')
                file_route = file_route.substr(1);
            if (file_route[file_route.size() - 1] == '/')
                file_route = file_route.substr(0, file_route.size() - 1);
            std::cout << "Route: " << file_route << std::endl;
        } else {
            std::cerr << "Route field not found!" << std::endl;
            code = 402;
        }
        boundaryMarker = "------WebKitFormBoundary";
        boundaryPos = body.find(boundaryMarker);

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
                body = body.substr(0, body.size() - 2);
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
        }
        else {
            std::cerr << "Boundary not found!" << std::endl;
            code = 415;
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
std::string Request::getFileRoute() const { return file_route; }
void Request::setCode(int code) { this->code = code; }
int Request::getServer() { return server; }
void Request::setServer(int server) { this->server = server; }
void Request::setUrl(std::string url) { this->url = url; }
bool Request::isValid() const { return valid; }