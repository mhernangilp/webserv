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
        std::string method;
        std::string url;
        std::string http_version;
        std::string host;
        std::map<std::string, std::string> headers;
        std::string body;
        std::string file_name;
        std::string file_route;
        int code;
        int server;
        bool valid;

    public:
    Request();
        Request(const std::string raw_request);
        Request(const Request& other);
        Request& operator=(const Request& other);

        std::string getMethod() const;
        std::string getUrl() const;
        std::string getHttpVersion() const;
        std::string getHost() const;
        std::map<std::string, std::string> getHeaders() const;
        std::string getBody() const;
        std::string getFileName() const;
        std::string getFileRoute() const;
        int getCode();
        void setCode(int code);
        int getServer();
        void setServer(int code);
        void setUrl(std::string url);
        bool isValid() const;

    private:
        void parseRequest(const std::string raw_request);
};

#endif
