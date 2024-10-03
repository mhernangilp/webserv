#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "../config/ServerConfig.hpp"
#include <fstream> 
#include <sstream>
#include <string> 
#include <iostream> 
#include <sys/socket.h>
#include <unistd.h>
#include <map>

class Request {
    private:
        std::string method;                  // Método HTTP (GET, POST, etc.)
        std::string url;                     // URL solicitada
        std::string http_version;            // Versión HTTP (ej. HTTP/1.1)
        std::string host;                    // Host (campo "Host" en los headers)
        std::map<std::string, std::string> headers; // Headers HTTP (otros campos como User-Agent, etc.)
        std::string body;                    // Cuerpo del mensaje (si hay uno)

    public:
    Request();
        Request(const std::string& raw_request);

        std::string getMethod() const;
        std::string getUrl() const;
        std::string getHttpVersion() const;
        std::string getHost() const;
        std::map<std::string, std::string> getHeaders() const;
        std::string getBody() const;

    private:
        void parseRequest(const std::string& raw_request);
};

void getResponse(const std::string& url, int client_socket, const ServerConfig& serverConfig);
void deleteResponse(const std::string& url, int client_socket, const ServerConfig& serverConfig);

#endif
